-- Markov Chain Program in Lua
    
function allwords ()
  local line = io.read()    -- current line
  local pos = 1             -- current position in the line
  return function ()        -- iterator function
    while line do           -- repeat while there are lines
      local s, e = string.find(line, "%w+", pos)
      if s then      -- found a word?
        pos = e + 1  -- update next position
        return string.sub(line, s, e)   -- return the word
      else
        line = io.read()    -- word not found; try next line
        pos = 1             -- restart from first position
      end
    end
    return nil            -- no more lines: end of traversal
  end
end

function prefix (w1, w2)
  return w1 .. ' ' .. w2
end

local statetab

function insert (index, value)
  if not statetab[index] then
    statetab[index] = {n=0}
  end
  table.insert(statetab[index], value)
end

local N  = 2
local MAXGEN = 10000
local NOWORD = "\n"

-- build table
statetab = {}
local w1, w2 = NOWORD, NOWORD
for w in allwords() do
  insert(prefix(w1, w2), w)
  w1 = w2; w2 = w;
end
insert(prefix(w1, w2), NOWORD)
-- generate text
w1 = NOWORD; w2 = NOWORD     -- reinitialize
for i=1,MAXGEN do
  local list = statetab[prefix(w1, w2)]
  -- choose a random item from list
  local r = math.random(table.getn(list))
  local nextword = list[r]
  if nextword == NOWORD then return end
  io.write(nextword, " ")
  w1 = w2; w2 = nextword
end
