; luademo.lisp
(define lua.allwords (lambda (yield) (block @func@
  (let (
    (lua.line ((index 'read lua.io) ())))
  (let (
    (lua.pos 1))
  (let ((ret (lambda (yield) (block @func@
    (block @loop@ (while lua.line
      (let ((lua.s) (lua.e))
      (assign (lua.s lua.e) (((index 'find lua.string) () lua.line "%w+" lua.pos)))
      (cond
        (lua.s
          (assign (lua.pos) ((+ lua.e 1)))
          (let ((ret ((index 'sub lua.string) () lua.line lua.s lua.e)))
          (if (and yield (not (null ret))) (yield ret) (return-from @func@ ret))))
        (#t
          (assign (lua.line) (((index 'read lua.io) ())))
          (assign (lua.pos) (1)))))))
    (let ((ret ()))
    (if (and yield (not (null ret))) (yield ret) (return-from @func@ ret)))))))
  (if (and yield (not (null ret))) (yield ret) (return-from @func@ ret))))))))
(define lua.prefix (lambda (yield lua.w1 lua.w2) (block @func@
  (let ((ret (.. lua.w1 " " lua.w2)))
  (if (and yield (not (null ret))) (yield ret) (return-from @func@ ret))))))
(let (
  (lua.statetab))
(define lua.insert (lambda (yield lua.index lua.value) (block @func@
  (cond
    ((not (index lua.index lua.statetab))
      (assign ((index lua.index lua.statetab)) ((list (cons 'lua.n 0))))))
  ((index 'insert lua.table) () (index lua.index lua.statetab) lua.value))))
(let (
  (lua.N 2))
(let (
  (lua.MAXGEN 10000))
(let (
  (lua.NOWORD "\x0a"))
(assign (lua.statetab) (()))
(let (
  (lua.w1 lua.NOWORD)
  (lua.w2 lua.NOWORD))
((lua.allwords ()) (lambda (lua.w)
  (lua.insert () (lua.prefix () lua.w1 lua.w2) lua.w)
  (assign (lua.w1) (lua.w2))
  (assign (lua.w2) (lua.w))))
(lua.insert () (lua.prefix () lua.w1 lua.w2) lua.NOWORD)
(assign (lua.w1) (lua.NOWORD))
(assign (lua.w2) (lua.NOWORD))
(block @loop@ (do ((lua.i 1 (+ lua.i 1)) ((> lua.i lua.MAXGEN))
  (let (
    (lua.list (index (lua.prefix () lua.w1 lua.w2) lua.statetab)))
  (let (
    (lua.r ((index 'random lua.math) () ((index 'getn lua.table) () lua.list))))
  (let (
    (lua.nextword (index lua.r lua.list)))
  (cond
    ((== lua.nextword lua.NOWORD)
      (let ((ret))
      (if (and yield (not (null ret))) (yield ret) (return-from @func@ ret)))))
  ((index 'write lua.io) () lua.nextword " ")
  (assign (lua.w1) (lua.w2))
  (assign (lua.w2) (lua.nextword)))))))))))))
