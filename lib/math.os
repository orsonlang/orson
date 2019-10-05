!
!  ORSON/LIB/MATH. Mathematical constants and functions.
!
!  Copyright © 2017 James B. Moen.
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

(load ''lib.C.math:c'')  !  Mathematical constants and functions.

!  Constants from /usr/include/math.h and /usr/include/bits/mathcalls.h.

(prog
  rej e               :− 2.7182818284590452354
  rej "log2(e)"       :− 1.4426950408889634074   !  Log base 2 of e.
  rej "log10(e)"      :− 0.43429448190325182765  !  Log base 10 of e.
  rej "loge(2.0)"     :− 0.69314718055994530942  !  Log base e of 2.
  rej "loge(10.0)"    :− 2.30258509299404568402  !  Log base e of 10.
  rej pi              :− 3.14159265358979323846
  rej "pi/2.0"        :− 1.57079632679489661923
  rej "pi/4.0"        :− 0.78539816339744830962
  rej "1.0/pi"        :− 0.31830988618379067154
  rej "2.0/pi"        :− 0.63661977236758134308
  rej "2.0/sqrt(pi)"  :− 1.12837916709551257390
  rej "sqrt(2.0)"     :− 1.41421356237309504880
  rej "1.0/sqrt(2.0)" :− 0.70710678118654752440

!  Fancy names for some of these constants.

  rej "log₂(e)"       :− "log2(e)"
  rej "log₁₀(e)"      :− "log10(e)"
  rej "logₑ(2.0)"     :− "loge(2.0)"
  rej "logₑ(10.0)"    :− "loge(10.0)"
  rej π               :− pi
  rej "π/2.0"         :− "pi/2.0"
  rej "π/4.0"         :− "pi/4.0"
  rej "1.0/π"         :− "1.0/pi"
  rej "2.0/π"         :− "2.0/pi"
  rej "2.0/√π"        :− "2.0/sqrt(pi)"
  rej "√2.0"          :− "sqrt(2.0)"
  rej "1.0/√2.0"      :− "1.0/sqrt(2.0)"

!  IRR. Return a form that calls NAME on an INT and a real, returning a real.

  IRR :−
   (form (string name) foj:
    (alt
     (form (int k, real0 y) real0:
       c's(real0, name + 'f': k, y)),
     (form (int k, real1 y) real1:
       c's(real1, name: k, y))))

!  RB. Return a form that calls NAME on a real, returning a BOOL.

  RB :−
   (form (string name) foj:
    (form (rej x) bool:
      c's(int, name: x) ≠ 0))

!  RI. Return a form that calls NAME on a real, returning an INT.

  RI :−
   (form (string name) foj:
    (alt
     (form (real0 x) int:
       c's(int, name & 'f': x)),
     (form (real1 x) int:
       c's(int, name: x))))

!  RR. Return a form that calls NAME on a real, returning a real.

  RR :−
   (form (string name) foj:
    (alt
     (form (real0 x) real0:
       c's(real0, name & 'f': x)),
     (form (real1 x) real1:
       c's(real1, name: x))))

!  RIR. Return a form that calls NAME on a real and an INT, returning a real.

  RIR :−
   (form (string name) foj:
    (alt
     (form (real0 x, int k) real0:
       c's(real0, name & 'f': x, k)),
     (form (real1 x, int k) real1:
       c's(real1, name: x, k))))

!  RRR. Return a form that calls NAME on two reals, returning a real.

  RRR :−
   (form (string name) foj:
    (alt
     (form (real0 x, real0 y) real0:
       c's(real0, name & 'f': x, y)),
     (form (real1 x, real1 y) real1:
       c's(real1, name: x, y))))

!  Forms that call functions from /usr/include/math.h. Note that ABS is defined
!  in the Standard Prelude.

  arcCos      :− RR(''acos'')         !  Inverse cosine.
  arcCosh     :− RR(''acosh'')        !  Inverse hyperbolic cosine.
  arcSin      :− RR(''asin'')         !  Inverse sine.
  arcSinh     :− RR(''asinh'')        !  Inverse hyperbolic sine.
  arcTan      :− RR(''atan'')         !  Inverse tangent.
  arcTan2     :− RRR(''atan2'')       !  Inverse tangent of Y / X.
  arcTanh     :− RR(''atanh'')        !  Inverse hyperbolic tangent.
  cbrt        :− RR(''cbrt'')         !  Cube root.
  ceil        :− RR(''ceil'')         !  Smallest integer less than X.
  copysign    :− RRR(''copysign'')    !  Copy sign of Y to X.
  cos         :− RR(''cos'')          !  Cosine.
  cosh        :− RR(''cosh'')         !  Hyperbolic cosine.
  erf         :− RR(''erf'')          !  Error function.
  erfc        :− RR(''erfc'')         !  Complementary error function.
  exp         :− RR(''exp'')          !  E to the power X.
  exp2        :− RR(''exp2'')         !  2.0 to the power X.
  exp₂        :− exp2                 !  2.0 to the power X.
  exp10       :− RR(''exp10'')        !  10.0 to the power X.
  exp₁₀       :− exp10                !  10.0 to the power X.
  expm1       :− RR(''expm1'')        !  Equivalent to exp(X − 1).
  floor       :− RR(''floor'')        !  Largest integer not greater than X.
  "mod"       :− RRR(''fmod'')        !  Real remainder after dividing X by Y.
  gamma       :− RR(''tgamma'')       !  Gamma function of X.
  Γ           :− gamma                !  Gamma function of X.
  hypot       :− RRR(''hypot'')       !  Equivalent to sqrt(X × X + Y × Y).
  isFinite    :− RB(''isfinite'')     !  Test if X is not NAN and not ∞.
  isInfinite  :− RB(''isinf'')        !  Test if X is ∞.
  isNan       :− RB(''isnan'')        !  Test if X is a NAN.
  isNormal    :− RB(''isnormal'')     !  Test if X is a normal real.
  j0          :− RR(''j0'')           !  Bessel function, 1st kind, order 0.
  j₀          :− j0                   !  Bessel function, 1st kind, order 0.
  j1          :− RR(''j1'')           !  Bessel function, 1st kind, order 1.
  j₁          :− j1                   !  Bessel function, 1st kind, order 1.
  jn          :− IRR(''jn'')          !  Bessel function, 1st kind, order N.
  ldexp       :− RIR(''ldexp'')       !  Multiply X by 2.0 to the power K.
  ln          :− RR(''log'')          !  Natural logarithm of X.
  log         :− ln                   !  Natural logarithm of X.
  log10       :− RR(''log10'')        !  Base 10 logarithm of X.
  log₁₀       :− log10                !  Base 10 logarithm of X.
  log1p       :− RR(''log1p'')        !  Equivalent to ln(1 + X).
  log2        :− RR(''log2'')         !  Base 2 logarithm of X.
  log₂        :− log2                 !  Base 2 logarithm of X.
  logb        :− RR(''logb'')         !  Exponent of X as a real.
  lround      :− RI(''lround'')       !  Round to nearest int, away from 0.
  pow         :− RRR(''pow'')         !  X to the power Y.
  round       :− RI(''round'')        !  Round to nearest int, away from 0.
  sign        :− RB(''signbit'')      !  Sign bit of X.
  significand :− RR(''significand'')  !  Mantissa of X, in [1.0, 2.0).
  sin         :− RR(''sin'')          !  Sine.
  sinh        :− RR(''sinh'')         !  Hyperbolic sine.
  sqrt        :− RR(''sqrt'')         !  Square root.
  tan         :− RR(''tan'')          !  Tangent.
  tanh        :− RR(''tanh'')         !  Hyperbolic tangent.
  trunc       :− RR(''trunc'')        !  Round X to int, toward 0.
  y0          :− RR(''y0'')           !  Bessel function, 2nd kind, order 0.
  y₀          :− y0                   !  Bessel function, 2nd kind, order 0.
  y1          :− RR(''y1'')           !  Bessel function, 2nd kind, order 1.
  y₁          :− y1                   !  Bessel function, 2nd kind, order 1.
  yn          :− IRR(''yn'')          !  Bessel function, 2nd kind, order N.

!  FREXP. Wrapper. Split X into a normalized fraction F and an exponent E.

  frexp :−
   (alt
    (form (real0 x) foj:
     (form (form (real0, int) obj body) obj:
      (with
        var int e
        real0   f :− c's(real0, ''frexpf'': x, e↓)
       do body(f, e)))),
    (form (real1 x) foj:
     (form (form (real1, int) obj body) obj:
      (with
        var int e
        real1 f :− c's(real1, ''frexp'': x, e↓)
       do body(f, e)))))

!  LGAMMA. Wrapper. Compute the BOOL sign of Γ(X), along with ln(abs(Γ(X))).

  lgamma :−
   (alt
    (form (real0 x) foj:
     (form (form (int, real0) obj body) obj:
      (with
        var int s
        real0   Γ :− c's(real0, ''lgammaf_r'': x, s↓)
        bool    b :− s ≠ 0
       do body(b, Γ)))),
    (form (real1 x) foj:
     (form (form (int, real1) obj body) obj:
      (with
        var int s
        real1   Γ :− c's(real1, ''lgamma_r'': x, s↓)
        bool    b :− s ≠ 0
       do body(b, Γ)))))

!  MODF. Wrapper. Split X into an integer part K and a fractional part F.

  modf :−
   (alt
    (form (real0 x) foj:
     (form (form (real0, real0) obj body) obj:
      (with
        var real0 k
        real0     f :− c's(real0, ''modff'': x, k↓)
       do body(k, f)))),
    (form (real1 x) foj:
     (form (form (real1, real1) obj body) obj:
      (with
        var real1 k
        real1     f :− c's(real1, ''modf'': x, k↓)
       do body(k, f)))))

!  SIN COS. Wrapper. Compute sin(X) and cos(X).

  sinCos :−
   (alt
    (form (real0 x) foj:
     (form (form (real0, real0) obj body) obj:
      (with
        var real0 sin
        var real0 cos
       do c's(void, ''sincosf'': x, sin↓, cos↓)
          body(sin, cos)))),
    (form (real1 x) foj:
     (form (form (real1, real1) obj body) obj:
      (with
        var real1 sin
        var real1 cos
       do c's(void, ''sincos'': x, sin↓, cos↓)
          body(sin, cos)))))

!  "␣{}". If E is a constant integer, then raise integer or real B to the power
!  E using inline multiplication. It's an error if E is not a constant. If both
!  E and B are reals, then raise E to B by calling POW (see above).

  " {}" :−
   (alt
    (gen (type num bType)
      form (bType b, inj e) bType:
       (if isInt(e)
        then (with
               bType b :− (past b)
               raising :−
                (form (inj e) bType:
                 (if e = 0
                  then 1{bType}
                  else if e = 1
                       then b
                       else if e & 1
                            then b × raising(e − 1)
                            else (with bType t :− raising(e / 2)
                                  do t × t)))
              do (if e < 0
                  then 1{bType} / raising(− e)
                  else raising(e)))
        else error($e, "constant expected")
             b)),
    pow)
)
