!
!  ORSON/LIB/PLAIN. Operations on Orson plain names.
!
!  Copyright © 2012 James B. Moen.
!
!  This  program is free  software: you  can redistribute  it and/or  modify it
!  under the terms  of the GNU General Public License as  published by the Free
!  Software Foundation,  either version 3 of  the License, or  (at your option)
!  any later version.
!
!  This program is distributed in the  hope that it will be useful, but WITHOUT
!  ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
!  FITNESS FOR  A PARTICULAR PURPOSE.  See  the GNU General  Public License for
!  more details.
!
!  You should have received a copy of the GNU General Public License along with
!  this program.  If not, see <http://www.gnu.org/licenses/>.
!

!  Most forms and procedures in this file test whether a CHAR can be part of an
!  Orson plain name. Some have Secret versions that test only the low byte of a
!  CHAR, for internal use only. Character names that appear in comments are the
!  "official" Unicode names.

(load ''lib.match'')  !  Test if a string is a member of a list of strings.

(prog

!  LB. Return the low byte of CH.

  Lb :−
   (form (char ch) int:
     16#FF & ch)

!  UB. Return the upper three bytes of CH.

  Ub :−
   (form (char ch) int:
     ch → 8)

!  IS PLAIN. Test if CH can be in a plain name.

  isPlain :−
   (proc (char ch) bool:
    (case Ub(ch)
     of Ub('a'): isPlainAscii(ch)
        Ub('α'): IsPlainGreek(ch)
        Ub('₀'): isPlainSubscript(ch)
        Ub('∀'): IsPlainOther(ch)
           none: false))

!  IS PLAIN ASCII. Test if CH is an ASCII char that can be in a plain name.

  isPlainAscii :−
   (form (char ch) bool:
     isPlainRoman(ch) ∨
     '0' ≤ ch ≤ '9' ∨
     '\'' = ch)

!  IS PLAIN GREEK. Test if CH is a Greek letter that can be in a plain name.

  isPlainGreek :−
   (proc (char ch) bool:
     Ub(ch) = Ub('α') ∧ IsPlainGreek(ch))

  IsPlainGreek :−
   (form (char ch) bool:
    (case Lb(ch)
     of Lb('Γ'): true  !  U+0393 Greek capital letter gamma.
        Lb('Δ'): true  !  U+0394 Greek capital letter delta.
        Lb('Θ'): true  !  U+0398 Greek capital letter theta.
        Lb('Λ'): true  !  U+039B Greek capital letter lamda [sic].
        Lb('Ξ'): true  !  U+039E Greek capital letter xi.
        Lb('Π'): true  !  U+03A0 Greek capital letter pi.
        Lb('Σ'): true  !  U+03A3 Greek capital letter sigma.
        Lb('Φ'): true  !  U+03A6 Greek capital letter phi.
        Lb('Ω'): true  !  U+03A9 Greek capital letter omega.
        Lb('α'): true  !  U+03B1 Greek small letter alpha.
        Lb('β'): true  !  U+03B2 Greek small letter beta.
        Lb('γ'): true  !  U+03B3 Greek small letter gamma.
        Lb('δ'): true  !  U+03B4 Greek small letter delta.
        Lb('ε'): true  !  U+03B5 Greek small letter epsilon.
        Lb('ζ'): true  !  U+03B6 Greek small letter zeta.
        Lb('η'): true  !  U+03B7 Greek small letter eta.
        Lb('θ'): true  !  U+03B8 Greek small letter theta.
        Lb('ι'): true  !  U+03B9 Greek small letter iota.
        Lb('κ'): true  !  U+03BA Greek small letter kappa.
        Lb('λ'): true  !  U+03BB Greek small letter lamda [sic].
        Lb('μ'): true  !  U+03BC Greek small letter mu.
        Lb('ν'): true  !  U+03BD Greek small letter nu.
        Lb('ξ'): true  !  U+03BE Greek small letter xi.
        Lb('π'): true  !  U+03C0 Greek small letter pi.
        Lb('ρ'): true  !  U+03C1 Greek small letter rho.
        Lb('ς'): true  !  U+03C2 Greek small letter final sigma.
        Lb('σ'): true  !  U+03C3 Greek small letter sigma.
        Lb('τ'): true  !  U+03C4 Greek small letter tau.
        Lb('υ'): true  !  U+03C5 Greek small letter upsilon.
        Lb('ϕ'): true  !  U+03C6 Greek small letter phi.
        Lb('χ'): true  !  U+03C7 Greek small letter chi.
        Lb('ψ'): true  !  U+03C8 Greek small letter psi.
        Lb('ω'): true  !  U+03C9 Greek small letter omega.
        Lb('ϑ'): true  !  U+03D1 Greek theta symbol.
        Lb('ϒ'): true  !  U+03D2 Greek upsilon with hook symbol.
        Lb('φ'): true  !  U+03D5 Greek phi symbol.
        Lb('ϖ'): true  !  U+03D6 Greek pi symbol.
        Lb('ϱ'): true  !  U+03F1 Greek rho symbol.
        Lb('ϵ'): true  !  U+03F5 Greek lunate epsilon symbol.
           none: false))

!  IS PLAIN LETTER. Test if CH is a letter that can be in a plain name. A plain
!  name must start with such a letter.

  isPlainLetter :−
   (proc (char ch) bool:
    (case Ub(ch)
     of Ub('a'): isPlainRoman(ch)
        Ub('α'): IsPlainGreek(ch)
        Ub('∀'): IsPlainOther(ch)
           none: false))

!  IS PLAIN OTHER. Test if CH is some other char that can be in a plain name.

  isPlainOther :−
   (proc (char ch) bool:
     Ub(ch) = Ub('∀') ∧ IsPlainOther(ch))

  IsPlainOther :−
   (form (char ch) bool:
    (case Lb(ch)
     of Lb('∀'): true  !  U+2200 for all.
        Lb('∃'): true  !  U+2203 there exists.
        Lb('∅'): true  !  U+2205 empty set.
        Lb('∞'): true  !  U+221E infinity.
        Lb('⊥'): true  !  U+22A5 up tack.
           none: false))

!  IS PLAIN ROMAN. Test if CH is a Roman letter that can be in a plain name.

  isPlainRoman :−
   (form (char ch) bool:
     'a' ≤ ch ≤ 'z' ∨
     'A' ≤ ch ≤ 'Z')

!  IS PLAIN SUBSCRIPT. Test if CH is a subscript that can be in a plain name.

  isPlainSubscript :−
   (form (char ch) bool:
     '₀' ≤ ch ≤ '₉')

!  IS RESERVED. Test if NAME is an Orson reserved (bold) name. We use a perfect
!  hash function that was constructed by trial and error.

  isReserved :−
   (proc (string k) bool:
    (with int l :− length(k)
     do 2 ≤ l ≤ 5 ∧
        k = (case (k[0] + 26 × k[l − 1] + 49 × l) mod 59
             of 52: ''also''  ; 42: ''gen''   ; 50: ''proc''
                15: ''alt''   ; 23: ''if''    ; 36: ''prog''
                38: ''alts''  ; 54: ''in''    ; 22: ''ref''
                12: ''and''   ; 13: ''load''  ; 51: ''row''
                30: ''case''  ; 24: ''mod''   ; 45: ''then''
                39: ''catch'' ; 41: ''none''  ; 37: ''tuple''
                16: ''do''    ; 28: ''not''   ; 47: ''type''
                32: ''else''  ; 29: ''of''    ; 43: ''var''
                27: ''for''   ; 46: ''or''    ; 40: ''while''
                05: ''form''  ; 20: ''past''  ; 10: ''with''
              none: ϵ)))
)
