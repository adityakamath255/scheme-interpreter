#include <builtins/preamble.hpp>
#include <string_view>

namespace Scheme {

const std::string_view preamble = R"(

(begin 
  (define (caar x) (car (car x)))
  (define (cadr x) (car (cdr x)))
  (define (cdar x) (cdr (car x)))
  (define (cddr x) (cdr (cdr x)))

  (define (caaar x) (car (car (car x))))
  (define (caadr x) (car (car (cdr x))))
  (define (cadar x) (car (cdr (car x))))
  (define (caddr x) (car (cdr (cdr x))))
  (define (cdaar x) (cdr (car (car x))))
  (define (cdadr x) (cdr (car (cdr x))))
  (define (cddar x) (cdr (cdr (car x))))
  (define (cdddr x) (cdr (cdr (cdr x))))

  (define (caaaar x) (car (car (car (car x)))))
  (define (caaadr x) (car (car (car (cdr x)))))
  (define (caadar x) (car (car (cdr (car x)))))
  (define (caaddr x) (car (car (cdr (cdr x)))))
  (define (cadaar x) (car (cdr (car (car x)))))
  (define (cadadr x) (car (cdr (car (cdr x)))))
  (define (caddar x) (car (cdr (cdr (car x)))))
  (define (cadddr x) (car (cdr (cdr (cdr x)))))
  (define (cdaaar x) (cdr (car (car (car x)))))
  (define (cdaadr x) (cdr (car (car (cdr x)))))
  (define (cdadar x) (cdr (car (cdr (car x)))))
  (define (cdaddr x) (cdr (car (cdr (cdr x)))))
  (define (cddaar x) (cdr (cdr (car (car x)))))
  (define (cddadr x) (cdr (cdr (car (cdr x)))))
  (define (cdddar x) (cdr (cdr (cdr (car x)))))
  (define (cddddr x) (cdr (cdr (cdr (cdr x)))))

  (define (map f lst)
    (if (null? lst)
        '()
        (cons (f (car lst))
              (map f (cdr lst)))))

  (define (filter pred lst)
    (if (null? lst)
        '()
        (if (pred (car lst))
            (cons (car lst) (filter pred (cdr lst)))
            (filter pred (cdr lst)))))
  
  (define (reduce f init lst)
    (if (null? lst)
        init
        (reduce f (f init (car lst)) (cdr lst))))

  (define (last-pair lst)
    (if (null? (cdr lst))
        lst
        (last-pair (cdr lst))))

  (define (append . lists)
    (cond ((null? lists) '())
          ((null? (cdr lists)) (car lists))
          ((null? (car lists)) (apply append (cdr lists)))
          (else (cons (car (car lists))
                      (apply append (cons (cdr (car lists))
                                          (cdr lists)))))))

  (define (append! . lists)
    (define (find-last-pair lst)
      (if (null? (cdr lst))
          lst
          (find-last-pair (cdr lst))))
  
  (cond ((null? lists) '())
        ((null? (cdr lists)) (car lists))
        (else
         (let ((first-list (car lists)))
           (if (null? first-list)
               (apply append! (cdr lists))
               (begin
                 (set-cdr! (find-last-pair first-list)
                           (apply append! (cdr lists)))
                 first-list))))))
  
  (define (reverse lst)
    (define (iter l acc)
      (if (null? l) 
          acc
          (iter (cdr l) (cons (car l) acc))))
    (iter lst '()))
  
  (define (memq obj lst)
    (if (null? lst)
        #f
        (if (eq? obj (car lst))
            lst
            (memq obj (cdr lst)))))
  
  (define (assoc key alist)
    (if (null? alist)
        #f
        (if (equal? key (car (car alist)))
            (car alist)
            (assoc key (cdr alist)))))
  
  (define (any pred lst)
    (if (null? lst)
        #f
        (if (pred (car lst))
            #t
            (any pred (cdr lst)))))
  
  (define (every pred lst)
    (if (null? lst)
        #t
        (if (pred (car lst))
            (every pred (cdr lst))
            #f)))
)

)";

}
