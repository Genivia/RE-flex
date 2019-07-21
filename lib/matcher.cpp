/******************************************************************************\
* Copyright (c) 2016, Robert van Engelen, Genivia Inc. All rights reserved.    *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
*   (1) Redistributions of source code must retain the above copyright notice, *
*       this list of conditions and the following disclaimer.                  *
*                                                                              *
*   (2) Redistributions in binary form must reproduce the above copyright      *
*       notice, this list of conditions and the following disclaimer in the    *
*       documentation and/or other materials provided with the distribution.   *
*                                                                              *
*   (3) The name of the author may not be used to endorse or promote products  *
*       derived from this software without specific prior written permission.  *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO   *
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, *
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;  *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,     *
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF       *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                   *
\******************************************************************************/

/**
@file      matcher.cpp
@brief     RE/flex matcher engine
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2015-2019, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include <reflex/matcher.h>

namespace reflex {

size_t Matcher::match(Method method)
{
  DBGLOG("BEGIN Matcher::match()");
  reset_text();
scan:
  mrk_ = false;
  txt_ = buf_ + cur_;
  len_ = 0;
  bool bob = at_bob();
  if (ded_ == 0 && hit_end() && tab_.empty())
  {
    if (method == Const::SPLIT && !bob && cap_ != 0 && cap_ != Const::EMPTY)
    {
      cap_ = Const::EMPTY;
      DBGLOG("Split empty at end, cap = %zu", cap_);
      DBGLOG("END Matcher::match()");
      return cap_;
    }
    cap_ = 0;
    DBGLOG("END Matcher::match()");
    return 0;
  }
  bool bol = bob || at_bol();
  bool bow;
  bool eow;
  int c1 = got_;
  if (isword(c1))
  {
    bow = false;
    eow = isword(peek()) == 0;
  }
  else
  {
    bow = isword(peek()) != 0;
    eow = false;
  }
  ind_ = pos_; // ind scans input in buf[] in newline() up to pos - 1
  size_t col = 0; // count columns from BOL
  if (pat_->fsm_)
  {
    fsm_.bob = bob;
    fsm_.bow = bow;
    fsm_.eow = eow;
    fsm_.col = col;
    fsm_.c1 = c1;
  }
redo:
  cap_ = 0;
  lap_.resize(0);
  bool nul = method == Const::MATCH;
  if (pat_->fsm_)
  {
    DBGLOG("FSM code %p", pat_->fsm_);
    fsm_.bol = bol;
    fsm_.nul = nul;
    pat_->fsm_(*this);
    col = fsm_.col;
    nul = fsm_.nul;
    c1 = fsm_.c1;
  }
  else if (pat_->opc_)
  {
    const Pattern::Opcode *pc = pat_->opc_;
    while (true)
    {
      Pattern::Opcode opcode = *pc;
      DBGLOG("Fetch: code[%zu] = 0x%08X", pc - pat_->opc_, opcode);
      Pattern::Index index;
      switch (opcode >> 16)
      {
        case 0x00ff: // check if HALT
          if (Pattern::is_opcode_halt(opcode))
            goto done;
          break;
        case 0xff00: // TAKE
          cap_ = Pattern::index_of(opcode);
          DBGLOG("Take: cap = %zu", cap_);
          cur_ = pos_;
          ++pc;
          continue;
        case 0xff7e: // TAIL
          index = Pattern::index_of(opcode);
          DBGLOG("Tail: %u", index);
          if (lap_.size() > index && lap_[index] >= 0)
            cur_ = txt_ - buf_ + static_cast<size_t>(lap_[index]); // mind the (new) gap
          ++pc;
          continue;
        case 0xff7f: // HEAD
          index = Pattern::index_of(opcode);
          DBGLOG("Head: lookahead[%u] = %zu", index, pos_ - (txt_ - buf_));
          if (lap_.size() <= index)
            lap_.resize(index + 1, -1);
          lap_[index] = static_cast<int>(pos_ - (txt_ - buf_)); // mind the gap
          ++pc;
          continue;
        case 0xff00 | Pattern::META_DED:
          if (ded_ > 0)
          {
            index = Pattern::index_of(opcode);
            DBGLOG("Dedent ded = %zu", ded_); // unconditional dedent matching \j
            nul = true;
            pc = pat_->opc_ + index;
            continue;
          }
      }
      int c0 = c1;
      if (c0 == EOF)
        break;
      c1 = get();
      DBGLOG("Get: c1 = %d", c1);
      index = Pattern::IMAX;
      Pattern::Index back = Pattern::IMAX; // where to jump back to (backtrack on meta transitions)
      Pattern::Index la;
      while (true)
      {
        if (index == Pattern::IMAX || back == Pattern::IMAX) // we no longer have to pass through all if index and back are set
        {
          switch (opcode >> 16)
          {
            case 0xff00: // TAKE
              cap_ = Pattern::index_of(opcode);
              DBGLOG("Take: cap = %zu", cap_);
              cur_ = pos_;
              if (c1 != EOF)
                --cur_; // Must unget one char
              opcode = *++pc;
              continue;
            case 0xff7e: // TAIL
              la = Pattern::index_of(opcode);
              DBGLOG("Tail: %u", la);
              if (lap_.size() > la && lap_[la] >= 0)
                cur_ = txt_ - buf_ + static_cast<size_t>(lap_[la]); // mind the (new) gap
              opcode = *++pc;
              continue;
            case 0xff7f: // HEAD
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_DED:
              DBGLOG("DED? %d", c1);
              if (index == Pattern::IMAX && back == Pattern::IMAX && bol && dedent(col))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_IND:
              DBGLOG("IND? %d", c1);
              if (index == Pattern::IMAX && back == Pattern::IMAX && bol && indent(col))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_UND:
              DBGLOG("UND");
              if (mrk_)
                index = Pattern::index_of(opcode);
              mrk_ = false;
              ded_ = 0;
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_EOB:
              DBGLOG("EOB? %d", c1);
              if (index == Pattern::IMAX && c1 == EOF)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_BOB:
              DBGLOG("BOB? %d", bob);
              if (index == Pattern::IMAX && bob)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_EOL:
              DBGLOG("EOL? %d", c1);
              if (index == Pattern::IMAX && (c1 == EOF || c1 == '\n'))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_BOL:
              DBGLOG("BOL? %d", bol);
              if (index == Pattern::IMAX && bol)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_EWE:
              DBGLOG("EWE? %d %d %d", c0, c1, isword(c0) && !isword(c1));
              if (index == Pattern::IMAX && isword(c0) && !isword(c1))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_BWE:
              DBGLOG("BWE? %d %d %d", c0, c1, !isword(c0) && isword(c1));
              if (index == Pattern::IMAX && !isword(c0) && isword(c1))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_EWB:
              DBGLOG("EWB? %d", eow);
              if (index == Pattern::IMAX && eow)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_BWB:
              DBGLOG("BWB? %d", bow);
              if (index == Pattern::IMAX && bow)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_NWE:
              DBGLOG("NWE? %d %d %d", c0, c1, isword(c0) == isword(c1));
              if (index == Pattern::IMAX && isword(c0) == isword(c1))
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
            case 0xff00 | Pattern::META_NWB:
              DBGLOG("NWB? %d %d", bow, eow);
              if (index == Pattern::IMAX && !bow && !eow)
                index = Pattern::index_of(opcode);
              opcode = *++pc;
              continue;
          }
        }
        if (index != Pattern::IMAX)
        {
          DBGLOG("Backtrack: pc = %u", index);
          if (back == Pattern::IMAX)
            back = static_cast<Pattern::Index>(pc - pat_->opc_);
          pc = pat_->opc_ + index;
          opcode = *pc;
          index = Pattern::IMAX;
        }
        else
        {
          if (back != Pattern::IMAX)
          {
            pc = pat_->opc_ + back;
            opcode = *pc;
          }
          break;
        }
      }
      if (c1 == EOF)
        break;
      Pattern::Opcode lo = c1 << 24;
      Pattern::Opcode hi = lo | 0x00ffffff;
unrolled:
      if (hi < opcode || lo > (opcode << 8))
      {
        opcode = *++pc;
        if (hi < opcode || lo > (opcode << 8))
        {
          opcode = *++pc;
          if (hi < opcode || lo > (opcode << 8))
          {
            opcode = *++pc;
            if (hi < opcode || lo > (opcode << 8))
            {
              opcode = *++pc;
              goto unrolled;
            }
          }
        }
      }
      index = Pattern::index_of(opcode);
      if (index == Pattern::IMAX)
        break;
      pc = pat_->opc_ + index;
    }
  }
done:
  if (mrk_ && cap_ != Const::EMPTY)
  {
    if (col > 0 && (tab_.empty() || tab_.back() < col))
    {
      DBGLOG("Set new stop: tab_[%zu] = %zu", tab_.size(), col);
      tab_.push_back(col);
    }
    else if (!tab_.empty() && tab_.back() > col)
    {
      size_t n;
      for (n = tab_.size() - 1; n > 0; --n)
        if (tab_.at(n - 1) <= col)
          break;
      ded_ += tab_.size() - n;
      DBGLOG("Dedents: ded = %zu tab_ = %zu", ded_, tab_.size());
      tab_.resize(n);
      if (n > 0)
        tab_.back() = col; // adjust stop when indents are not aligned (Python would give an error)
    }
  }
  if (ded_ > 0)
  {
    DBGLOG("Dedents: ded = %zu", ded_);
    if (col == 0 && bol)
    {
      ded_ += tab_.size();
      tab_.resize(0);
      DBGLOG("Rescan for pending dedents: ded = %zu", ded_);
      pos_ = ind_;
      bol = false; // avoid looping, match \j exactly
      goto redo;
    }
    --ded_;
  }
  if (method == Const::SPLIT)
  {
    DBGLOG("Split: len = %zu cap = %zu cur = %zu pos = %zu end = %zu txt-buf = %zu eob = %d got = %d", len_, cap_, cur_, pos_, end_, txt_-buf_, (int)eof_, got_);
    if (cap_ == 0 || (cur_ == static_cast<size_t>(txt_ - buf_) && !bob))
    {
      if (!hit_end())
      {
        ++len_;
        DBGLOG("Split continue: len = %zu", len_);
        set_current(++cur_);
        goto redo;
      }
      if (got_ != Const::EOB)
      {
        cap_ = Const::EMPTY;
        set_current(pos_); // chr_ = static_cast<unsigned char>(buf_[pos_]);
        got_ = Const::EOB;
      }
      DBGLOG("Split at eof: cap = %zu txt = '%s' len = %zu", cap_, std::string(txt_, len_).c_str(), len_);
      DBGLOG("END Matcher::match()");
      return cap_;
    }
    if (bob && cur_ == 0 && hit_end())
      cap_ = Const::EMPTY;
    set_current(cur_);
    DBGLOG("Split: txt = '%s' len = %zu", std::string(txt_, len_).c_str(), len_);
    DBGLOG("END Matcher::match()");
    return cap_;
  }
  if (cap_ == 0)
    cur_ = txt_ - buf_; // no match: backup to begin of text
  len_ = cur_ - (txt_ - buf_);
  if (len_ == 0 && !nul)
  {
    DBGLOG("Empty match cur = %zu pos = %zu end = %zu", cur_, pos_, end_);
    pos_ = cur_;
    if (hit_end())
    {
      set_current(cur_);
      DBGLOG("Reject empty match at EOF");
      if (!opt_.N || !bob) // allow FIND and SCAN with "N" to match empty input, with ^$ etc.
        cap_ = 0;
    }
    else if (method == Const::FIND)
    {
      set_current(++cur_); // skip one char to ensure we're advancing later
      DBGLOG("Reject empty match and continue?");
      if (cap_ == 0 || !opt_.N || (!bol && c1 == '\n')) // allow FIND with "N" to match an empty line, with ^$ etc.
        goto scan;
      DBGLOG("Accept empty match");
    }
    else
    {
      set_current(cur_);
      DBGLOG("Reject empty match");
      cap_ = 0;
    }
  }
  else if (cur_ == end_ && len_ == 0)
  {
    DBGLOG("Hit end: got = %d", got_);
    if (cap_ == Const::EMPTY && !opt_.A)
      cap_ = 0; // cannot goto scan?
  }
  else
  {
    set_current(cur_);
    if (len_ > 0)
    {
      if (cap_ == Const::EMPTY && !opt_.A)
      {
        DBGLOG("Ignore accept and continue: len = %zu", len_);
        if (method != Const::MATCH)
          goto scan;
        cap_ = 0;
      }
    }
  }
  DBGLOG("Return: cap = %zu txt = '%s' len = %zu pos = %zu got = %d", cap_, std::string(txt_, len_).c_str(), len_, pos_, got_);
  DBGLOG("END match()");
  return cap_;
}

} // namespace reflex
