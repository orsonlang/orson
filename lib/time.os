!
!  ORSON/LIB/TIME. Operations on dates and times.
!
!  Copyright © 2018 James B. Moen.
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

!  The form DECODED was suggested by the Common Lisp function GET-DECODED-TIME.
!  See:
!
!  Steele, Guy L. Jr. Common Lisp: The Language. Second Edition. Digital Press,
!  Bedford, Massachusetts. 1989. Pages 703–704.
!
!  Currently, names of days and months are available only in English. Sorry: we
!  ought to support other languages too. People bothered by this should use the
!  first member of DECODED with their own DAY, MERIDIEM, and MONTH forms.

(load ''lib.C.time:c'')  !  Date and time.

(prog

!  INSTANT. An instant of time. All we'll use here is the SECOND slot. See "man
!  ftime" for more information.

  instant :−
   (tuple
     int2 second,    !  Seconds since January 1, 1970, 00:00:00 UTC.
     int1 μsecond,   !  More precision for SECOND. It's less than 1000.
     int1 zone,      !  Minutes west of Greenwich.
     int1 daylight)  !  Is daylight saving time in effect?

!  NOW. Return an INSTANT denoting the current time. It's claimed that FTIME is
!  obsolete, but there's no alternative that works on all systems.

  now :−
   (form () instant:
    (with var instant temp
     do c's(int, ''ftime'': temp↓)
        temp))

!  DECODED. Wrapper. Break an INSTANT into its component parts in various ways,
!  and call BODY on those parts. For example, this code fragment will write the
!  current date and time as in "Tuesday, May 13, 2008 at 2:44 PM CDT".
!
!    (for
!      int second, int minute, int hour, string meridiem,
!      int date, string month, int year, string day, string zone
!     in decoded(now())
!     do writeln(
!         ''%s, %s %i, %i at %i:%02i %s %s'':
!         day, month, date, year, hour, minute, meridiem, zone))
!
!  Strings (like MONTH) are guaranteed to exist only while DECODED is running.

  decoded :−
   (with

!  BROKEN. The SECOND slot of an INSTANT, split into its component parts.

     broken :−
      ref
       (tuple
         int second,    !  Seconds after the minute, 0 thru 61.
         int minute,    !  Minutes after the hour, 0 thru 59.
         int hour,      !  Hours past midnight, 0 thru 23.
         int date,      !  Day of the month, 1 thru 31.
         int month,     !  Months since January, 0 thru 11.
         int year,      !  Years since 1900.
         int weekday,   !  Days since Sunday, 0 thru 6.
         int day,       !  Days since January 1, 0 thru 365.
         int daylight)  !  0 means no DST, > 0 means DST, < 0 means unknown.

!  CIVILIAN HOUR. Convert HOUR to "civilian" time, 1 through 12 inclusive.

     civilianHour :−
      (form (int hour) int:
       (with int temp :− hour mod 12
        do (if temp = 0
            then 12
            else temp)))

!  I, S. Short names for INT and STRING, used below in long parameter lists.

     i :− int
     s :− string

!  TIME ZONE. Return a string denoting the time zone. TZNAME is set by the call
!  to LOCALTIME below. We cast to VOID * because Orson represents pointers that
!  way when it translates to GCC. What a kludge.

     timeZone :−
      (form (int daylight) string:
       (if daylight < 0
        then ϵ
        else c's(row string, ''(void *)tzname'')[daylight > 0]))

!  We provide three ways to break NOW up into components:

    do (form (instant now) foj:
        (alt

!  The first way just calls BODY on the unchanged components. This is like what
!  Common Lisp's GET-DECODED-TIME does.

         (form (form (i, i, i, i, i, i, i, i, i) obj body) obj:
          (with
            instant now      :− (past now)
            broken  time     :− c's(broken, ''localtime'': now.second↓)
            int     second   :− time↑.second
            int     minute   :− time↑.minute
            int     hour     :− time↑.hour
            int     date     :− time↑.date
            int     month    :− time↑.month
            int     year     :− time↑.year
            int     weekday  :− time↑.weekday
            int     day      :− time↑.day
            int     daylight :− time↑.daylight
           do body(
               second, minute, hour,
               date, month, year, weekday, day, daylight))),

!  The second way is like the first, but it provides English strings for MONTH,
!  DAY, and the time ZONE.

         (form (form (i, i, i, i, s, i, s, s) obj body) obj:
          (with
            instant now    :− (past now)
            broken  time   :− c's(broken, ''localtime'': now.second↓)
            int     second :− time↑.second
            int     minute :− time↑.minute
            int     hour   :− time↑.hour
            int     date   :− time↑.date
            string  month  :− Month(time↑.month)
            int     year   :− time↑.year + 1900
            string  day    :− Day(time↑.weekday)
            string  zone   :− timeZone(time↑.daylight)
           do body(
               second, minute, hour,
               date, month, year, day, zone))),

!  The third way is like the second, but it converts 24-hour "military" time to
!  12-hour "civilian" time.

         (form (form (i, i, i, s, i, s, i, s, s) obj body) obj:
          (with
            instant now      :− (past now)
            broken  time     :− c's(broken, ''localtime'': now.second↓)
            int     second   :− time↑.second
            int     minute   :− time↑.minute
            int     hour     :− civilianHour(time↑.hour)
            string  meridiem :− Meridiem(time↑.hour < 12)
            int     date     :− time↑.date
            string  month    :− Month(time↑.month)
            int     year     :− time↑.year + 1900
            string  day      :− Day(time↑.weekday)
            string  zone     :− timeZone(time↑.daylight)
           do body(
               second, minute, hour, meridiem,
               date, month, year, day, zone))))))

!  DAY. Return names of the weekdays in English.

  Day :−
   (form (inj day) string:
    (case day
     of 0: ''Sunday''
        1: ''Monday''
        2: ''Tuesday''
        3: ''Wednesday''
        4: ''Thursday''
        5: ''Friday''
        6: ''Saturday''
     none: ϵ))

!  MERIDIEM. Return ante-meridiem or post-meridiem in English.

  Meridiem :−
   (form (bool morning) string:
    (if morning
     then ''AM''
     else ''PM''))

!  MONTH. Return names of the months in English.

  Month :−
   (form (inj month) string:
    (case month
     of 00: ''January''
        01: ''February''
        02: ''March''
        03: ''April''
        04: ''May''
        05: ''June''
        06: ''July''
        07: ''August''
        08: ''September''
        09: ''October''
        10: ''November''
        11: ''December''
      none: ϵ))
)
