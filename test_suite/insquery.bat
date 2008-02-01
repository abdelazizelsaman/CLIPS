(reset)
(do-for-all-instances ((?i OBJECT)) TRUE
   (+ (eval "(gensym)") 2))
(do-for-all-instances ((?mb MALE) (?wg FEMALE))
   TRUE
   (printout t ?mb " " ?wg crlf))
(do-for-all-instances ((?mb BOY MAN) (?wg GIRL WOMAN))
   TRUE
   (progn (printout t ?mb ) (printout t " " ?wg crlf)))
(do-for-all-instances ((?f1 FEMALE) (?f2 FEMALE))
   TRUE
   (printout t ?f1 " " ?f2 crlf))
(do-for-all-instances ((?mb MALE) (?wg FEMALE))
   (= (send ?mb get-age) (send ?wg get-age))
   (printout t ?mb " " ?wg crlf))
(do-for-all-instances ((?mb MALE) (?wg FEMALE))
   (= ?mb:age ?wg:age)
   (printout t ?mb " " ?wg crlf))
(count-instances WOMAN)
(count-instances-2 WOMAN)
(count-instances BOY)
(count-instances-2 BOY)
(any-instancep ((?man MAN)) (> ?man:age 30))
(find-instance ((?m MAN) (?w WOMAN)) (= ?m:age ?w:age))
(find-all-instances ((?m MAN) (?w WOMAN)) (= ?m:age ?w:age))
(do-for-instance ((?p1 PERSON) (?p2 PERSON) (?p3 PERSON))
  (and (= ?p1:age ?p2:age ?p3:age)
       (neq ?p1 ?p2)
       (neq ?p1 ?p3)
       (neq ?p2 ?p3))
  (printout t ?p1 " " ?p2 " " ?p3 crlf))
(do-for-all-instances ((?p1 PERSON) (?p2 PERSON) (?p3 PERSON))
  (and (= ?p1:age ?p2:age ?p3:age)
       (> (str-compare ?p1 ?p2) 0)
       (> (str-compare ?p2 ?p3) 0))
  (printout t ?p1 " " ?p2 " " ?p3 crlf))
(do-for-all-instances ((?p1 PERSON) (?p2 PERSON) (?p3 PERSON))
  (= ?p1:age ?p2:age ?p3:age)
  (printout t ?p1 " " ?p2 " " ?p3 crlf))
(watch instances)
(delayed-do-for-all-instances ((?b1 BOY))
   (not (any-instancep ((?b2 BOY))
           (> ?b2:age (send ?b1 get-age))))
   (send ?b1 delete))
(reset)
(do-for-all-instances ((?b1 BOY))
   (not (any-instancep ((?b2 BOY))
           (> ?b2:age (send ?b1 get-age))))
   (send ?b1 delete))
(unwatch instances)
(reset)
(do-for-all-instances ((?b BOY)) TRUE
  (do-for-all-instances ((?g GIRL)) TRUE
    (printout t ?b " " ?g crlf)))
(make-instance d of D)
(do-for-all-instances ((?a A)) TRUE
  (printout t ?a crlf))
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
(do-for-all-instances ((?a D) (?b D) (?c D) (?d D) (?e D) (?f D) (?g D))
     TRUE 
  (printout t "Should get a nesting error!" crlf
              "The allowed nesting level was increased," crlf
              "So this error hasn't been duplicated recently." crlf)
))))))))))))))))))))
(any-instancep)
(any-instancep TRUE)
(any-instancep ())
(any-instancep (?B OBJECT) TRUE)
(any-instancep ((?b A)))
(any-instancep ((?b BOGUS)) TRUE)
(any-instancep ((?b 34)) TRUE)
(any-instancep ((?b (mv-append BOGUS USER))) TRUE)
(any-instancep ((?b (mv-append USER 34))) TRUE)
(any-instancep ((?b A)) (progn (bind ?b 1) TRUE))
(find-instance)
(find-instance TRUE)
(find-instance ())
(find-instance (?B OBJECT) TRUE)
(find-instance ((?b A)))
(find-instance ((?b BOGUS)) TRUE)
(find-instance ((?b 34)) TRUE)
(find-instance ((?b (mv-append BOGUS USER))) TRUE)
(find-instance ((?b (mv-append USER 34))) TRUE)
(find-instance ((?b A)) (progn (bind ?b 1) TRUE))
(find-all-instances)
(find-all-instances TRUE)
(find-all-instances ())
(find-all-instances (?B OBJECT) TRUE)
(find-all-instances ((?b A)))
(find-all-instances ((?b BOGUS)) TRUE)
(find-all-instances ((?b 34)) TRUE)
(find-all-instances ((?b (mv-append BOGUS USER))) TRUE)
(find-all-instances ((?b (mv-append USER 34))) TRUE)
(find-all-instances ((?b A)) (progn (bind ?b 1) TRUE))
(do-for-instance)
(do-for-instance TRUE)
(do-for-instance ())
(do-for-instance (?B OBJECT) TRUE)
(do-for-instance ((?b A)))
(do-for-instance ((?b BOGUS)) TRUE)
(do-for-instance ((?b A)) (progn (bind ?b 1) TRUE))
(do-for-instance ((?b 34)) TRUE abc)
(do-for-instance ((?b (mv-append BOGUS USER))) TRUE abc)
(do-for-instance ((?b (mv-append USER 34))) TRUE abc)
(do-for-instance ((?b OBJECT)) TRUE (bind ?b 1))
(do-for-all-instances)
(do-for-all-instances TRUE)
(do-for-all-instances ())
(do-for-all-instances (?B OBJECT) TRUE)
(do-for-all-instances ((?b A)))
(do-for-all-instances ((?b BOGUS)) TRUE)
(do-for-all-instances ((?b A)) (progn (bind ?b 1) TRUE))
(do-for-all-instances ((?b 34)) TRUE abc)
(do-for-all-instances ((?b (mv-append BOGUS USER))) TRUE abc)
(do-for-all-instances ((?b (mv-append USER 34))) TRUE abc)
(do-for-all-instances ((?b OBJECT)) TRUE (bind ?b 1))
(delayed-do-for-all-instances)
(delayed-do-for-all-instances TRUE)
(delayed-do-for-all-instances ())
(delayed-do-for-all-instances (?B OBJECT) TRUE)
(delayed-do-for-all-instances ((?b A)))
(delayed-do-for-all-instances ((?b BOGUS)) TRUE)
(delayed-do-for-all-instances ((?b A)) (progn (bind ?b 1) TRUE))
(delayed-do-for-all-instances ((?b 34)) TRUE abc)
(delayed-do-for-all-instances ((?b (mv-append BOGUS USER))) TRUE abc)
(delayed-do-for-all-instances ((?b (mv-append USER 34))) TRUE abc)
(delayed-do-for-all-instances ((?b OBJECT)) TRUE (bind ?b 1))
(make-instance v of V)
(make-instance w of W)
(make-instance x of X)
(make-instance y of Y)
(make-instance z of Z)
(do-for-all-instances ((?ins1 ?*list* X (mv-append Y Z)) 
                       (?ins2 X (mv-append Y Z) ?*list*))
  TRUE
  (printout t ?ins1 " " ?ins2 crlf))
(while TRUE do
   (delayed-do-for-all-instances ((?b OBJECT)) TRUE 
     (progn (printout t DELAYED ?b crlf) (break)))
   (do-for-all-instances ((?b OBJECT)) TRUE 
     (progn (printout t DO-FOR-ALL ?b crlf) (break)))
   (break))
(deffunction quoxnar ()
   (do-for-all-instances ((?b OBJECT)) TRUE 
     (progn (printout t INS-RETURN ?b crlf) (return))))
(quoxnar)
