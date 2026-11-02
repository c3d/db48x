# Navigation functions

## NavigationLibrary

The Navigation section of the Function Library computes positions, distances
and directions on the Earth, for air, marine and land navigation.

The [Geodesy](#geodesylibrary) subsection computes on the Earth itself:

* On the **WGS-84 ellipsoid**, the reference surface of GPS, to the millimetre:
  [VincentyInv](#vincentyinv) gives the distance and azimuths between two
  points, [VincentyDir](#vincentydir) the point reached from a given point,
  azimuth and distance, and [LLH→XYZ](#llh→xyz) and [XYZ→LLH](#xyz→llh)
  convert GPS coordinates to and from Earth-centred Cartesian coordinates.
* On **map grids**: [LL→UTM](#ll→utm) and [UTM→LL](#utm→ll) convert to and from
  UTM coordinates, [LL→MGRS](#ll→mgrs) and [MGRS→LL](#mgrs→ll) to and from
  military grid references.
* On **tracks and areas**: [TrackLen](#tracklen) sums a track, and
  [PolyArea](#polyarea) gives the area of a polygon.
* In a **local frame**: [LLH→ENU](#llh→enu) and [ENU→LLH](#enu→llh).
* On the **magnetic field**: [WMM](#wmm) gives the magnetic declination at a point
  and date.
* On **time**: [UTC→GPS](#utc→gps) and [GPS→UTC](#gps→utc).
* As **text**: [LL→DMS](#ll→dms) and [LL→DDM](#ll→ddm) format a position.
* On a **sphere**, where the formulas are shorter and angles are enough:
  [RadialFix](#radialfix) finds a position from two bearings, and
  [GCPoint](#gcpoint) a point along a great-circle route.

The [Route](#routelibrary) subsection keeps a route of waypoints and follows
it while travelling.

**Conventions.** Latitudes are positive North, longitudes positive East, as in
GPS coordinates: West longitudes are negative. Azimuths and bearings are
measured clockwise from true North. Plain numbers are read as degrees and
metres; angles and lengths with a unit are accepted as well, for instance
`'33_°+57_arcmin'`. Results carry their units.

The functions work in radians internally; the angle mode and the angle units
setting are left as they were found.

The ellipsoid functions use the constants `Ⓒa♁GPS`, `Ⓒf♁GPS`, `Ⓒe12♁GPS` and
`Ⓒe22♁GPS`. The `Navigation` section of the Equation Library holds the
spherical equations of great circles and of the wind and current triangles.


## GeodesyLibrary

The Geodesy subsection computes distances, directions and positions:

* on the WGS-84 ellipsoid: [VincentyInv](#vincentyinv),
  [VincentyDir](#vincentydir), [LLH→XYZ](#llh→xyz) and [XYZ→LLH](#xyz→llh);
* on map grids of the same ellipsoid: [LL→UTM](#ll→utm), [UTM→LL](#utm→ll),
  [LL→MGRS](#ll→mgrs) and [MGRS→LL](#mgrs→ll);
* for tracks and areas: [TrackLen](#tracklen) and [PolyArea](#polyarea);
* in a local frame: [LLH→ENU](#llh→enu) and [ENU→LLH](#enu→llh);
* for the magnetic field: [WMM](#wmm);
* for GPS time: [UTC→GPS](#utc→gps) and [GPS→UTC](#gps→utc);
* as text: [LL→DMS](#ll→dms) and [LL→DDM](#ll→ddm);
* on a sphere: [RadialFix](#radialfix) and [GCPoint](#gcpoint).


## VincentyInv

The inverse geodetic problem: the distance between two points of the WGS-84
ellipsoid, and the azimuth of the shortest route at each end.

![Geodesic route](img/GreatCircleRoute.bmp)

Stack: `φ1` `λ1` `φ2` `λ2` ▶ `s` `α1` `α2`, where `s` is the distance along the
ellipsoid, `α1` the azimuth at departure and `α2` the azimuth on arrival, both
from true North.

The method is T. Vincenty's iteration (Survey Review 23(176), 1975), which
converges to about 0.006 mm. It does not converge for nearly antipodal points,
about 20 000 km apart: the function then stops with an error rather than return
a wrong value.

From Flinders Peak to Buninyong, in Australia, the classic test of
Geoscience Australia: 54 972.271 m, azimuth 306°52′05.37″:

```rpl
-37.95103341666667 144.42486788888889 -37.65282113888889 143.92649552777778
ⓁVincentyInv 3 →LIST
@ Expecting { 54 972.27113 87 m 306.86815 9203 ° 307.17363 0629 ° }
```

From New York (JFK, 40°38′23″N, 73°46′44″W) to Singapore (Changi, 1°21′33″N,
103°59′22″E); GeographicLib gives 15 347 628 m and 3°18′29.9″:

```rpl
'40_°+38_arcmin+23_arcs' '-(73_°+46_arcmin+44_arcs)'
'1_°+21_arcmin+33_arcs' '103_°+59_arcmin+22_arcs'
4 →LIST →Num LIST→ DROP
ⓁVincentyInv 3 →LIST
@ Expecting { 15 347 627.6596 m 3.30831 24000 9 ° 177.48590 2144 ° }
```


## VincentyDir

The direct geodetic problem: the point reached on the WGS-84 ellipsoid from a
given point, initial azimuth and distance.

Stack: `φ1` `λ1` `α1` `s` ▶ `φ2` `λ2` `α2`, where `α2` is the azimuth on arrival.

From 40°N on the prime meridian, 10 000 km on azimuth 30°; C. F. F. Karney
(Journal of Geodesy 87, 2013) gives 41.793310205°, 137.844900044° and
149.090169318°:

```rpl
40 0 30 10000_km ⓁVincentyDir 3 →LIST
@ Expecting { 41.79331 0205 ° 137.84490 0044 ° 149.09016 9318 ° }
```


## LLH→XYZ

Converts a GPS position — latitude, longitude and height above the WGS-84
ellipsoid — to Earth-centred, Earth-fixed (ECEF) coordinates. `X` points to
the equator on the prime meridian, `Y` to the equator at 90°E, `Z` to the North
pole.

![ECEF and geodetic coordinates](img/EcefCoordinates.bmp)

Stack: `φ` `λ` `h` ▶ `X` `Y` `Z`.

The example of IOGP Guidance Note 7-2, §4.1.1 (EPSG method 9602): 53°48′33.820″N,
2°07′46.380″E, 73 m give 3 771 793.968 m, 140 253.342 m and 5 124 304.349 m:

```rpl
'53_°+48_arcmin+33.820_arcs' '2_°+7_arcmin+46.380_arcs' 2 →LIST →Num LIST→ DROP
73_m ⓁLLH→XYZ 3 →LIST
@ Expecting { 3 771 793.96764 m 140 253.3419 m 5 124 304.34935 m }
```


## XYZ→LLH

Converts Earth-centred, Earth-fixed (ECEF) coordinates to latitude, longitude
and height above the WGS-84 ellipsoid, by the method of B. R. Bowring (Survey
Review 23(181), 1976), accurate to better than a millimetre near the Earth's
surface.

Stack: `X` `Y` `Z` ▶ `φ` `λ` `h`.

The example of LLH→XYZ in reverse; the coordinates, rounded to the
millimetre, give back 53°48′33.820″N, 2°07′46.380″E and 73 m:

```rpl
3771793.968 140253.342 5124304.349 ⓁXYZ→LLH 3 →LIST
@ Expecting { 53.80939 444 ° 2.12955 00013 2 ° 72.99993 06719 m }
```

See also: [LLH→XYZ](#llh→xyz), the conversion in the other direction.


## RadialFix

The position where two radials cross: the bearings of the same point taken
from two known stations, for instance two VOR beacons, or two landmarks seen
from a ship. The Earth is taken as a sphere.

![Fix by two radials](img/RadialFix.bmp)

Stack: `φ1` `λ1` `C1` `φ2` `λ2` `C2` ▶ `φ` `λ`, where `C1` and `C2` are the true
bearings of the sought position from the first and the second station.

The function stops with an error when the two radials lie on the same great
circle ("Infinity of intersections"), or when they diverge, so that their great
circles only meet on the far side of the Earth ("Intersection ambiguous").

From Ed Williams' *Aviation Formulary*: the radial 051° from Rome (REO,
42.600°N, 117.866°W) and the radial 137° from Baker City (BKE, 44.840°N,
117.806°W) cross over Boise (BOI, 43.572°N, 116.189°W):

```rpl
42.600 -117.866 51 44.840 -117.806 137 ⓁRadialFix 2 →LIST
@ Expecting { 43.57190 03837 ° -116.18875 7484 ° }
```


## GCPoint

A point along the great circle between two points of a sphere, at a given
fraction of the route: 0 at departure, 1 at destination. Following a great
circle usually means flying or steering straight legs between such points.

![Great circle route](img/GreatCircleRoute.bmp)

Stack: `φ1` `λ1` `φ2` `λ2` `f` ▶ `φ` `λ`.

It is not defined for antipodal points, between which every great circle is a
shortest route.

Four tenths of the way from Los Angeles (LAX, 33°57′N, 118°24′W) to New York
(JFK, 40°38′N, 73°47′W); Ed Williams gives 38°40.167′N, 101°37.570′W:

```rpl
33.95 -118.4 40.63333333333333 -73.78333333333333 0.4 ⓁGCPoint 2 →LIST
@ Expecting { 38.66944 7748 ° -101.62616 0313 ° }
```


## LL→UTM

Converts a latitude and longitude on the WGS-84 ellipsoid to Universal Transverse
Mercator coordinates, the metric grid of many topographic maps and GPS receivers.
The Earth is cut into 60 zones, each 6° wide, and each zone is projected on its
own plane, with an easting of 500 000 m on its central meridian.

Stack: `φ` `λ` ▶ `zone` `hem` `east` `north`, where `hem` is the text `"N"` or
`"S"`. In the southern hemisphere, the northing counts from 10 000 000 m at the
equator, so that it is never negative.

The latitude must lie from 80°S to 84°N, the range of UTM; beyond it, the function
stops with an error. The zones follow NGA.SIG.0012, with the wider zones of western
Norway and of Svalbard. The method is the Krüger series in the form of C. F. F.
Karney (Journal of Geodesy 85, 2011), to order 4, accurate to far less than a
millimetre over a zone.

In Montréal; GeographicLib gives zone 18 North, 609 377.347 m and 5 039 449.713 m:

```rpl
45.5 -73.6 ⓁLL→UTM 4 →LIST
@ Expecting { 18 "N" 609 377.34742 5 m 5 039 449.713 m }
```

In Sydney, in the southern hemisphere; GeographicLib gives zone 56 South,
334 416.394 m and 6 251 925.360 m:

```rpl
-33.86 151.21 ⓁLL→UTM 4 →LIST
@ Expecting { 56 "S" 334 416.39399 m 6 251 925.36035 m }
```

In Svalbard, zones 32, 34 and 36 do not exist: 15.6°E falls in zone 33, which
spans 9°E to 21°E; GeographicLib gives 513 696.945 m and 8 680 760.053 m:

```rpl
78.2 15.6 ⓁLL→UTM 4 →LIST
@ Expecting { 33 "N" 513 696.94541 7 m 8 680 760.0532 m }
```


## UTM→LL

Converts Universal Transverse Mercator coordinates to a latitude and longitude on
the WGS-84 ellipsoid.

Stack: `zone` `hem` `east` `north` ▶ `φ` `λ`, where `hem` is the text `"N"` or `"S"`.
Plain numbers are metres.

The coordinates of Sydney in the previous page, rounded to the millimetre, give back
33.86°S, 151.21°E:

```rpl
56 "S" 334416.394 6251925.360 ⓁUTM→LL 2 →LIST
@ Expecting { -33.86000 00032 ° 151.21 ° }
```


## LL→MGRS

Converts a latitude and longitude on the WGS-84 ellipsoid to a Military Grid
Reference System text. It names the UTM zone, a latitude band, a square of 100 km by
two letters, then the position inside that square. The system is also used by search
and rescue services and by the United States National Grid.

Stack: `φ` `λ` `n` ▶ `text`, where `n`, from 0 to 5, is the number of digits of each
coordinate: 5 names a square of 1 m, 4 of 10 m, 0 of 100 km.

As NGA.SIG.0012 requires, the digits are truncated, not rounded: the text names the
square that holds the point.

In Montréal, to the metre, then to 100 km; GeographicLib gives 18TXR 09377 39449:

```rpl
45.5 -73.6 5 ⓁLL→MGRS
@ Expecting "18T XR 09377 39449"
```

```rpl
45.5 -73.6 0 ⓁLL→MGRS
@ Expecting "18T XR"
```

In western Norway, where zone 32 is widened; GeographicLib gives 32VKN 96191 01684:

```rpl
60.4 5.3 5 ⓁLL→MGRS
@ Expecting "32V KN 96191 01684"
```


## MGRS→LL

Converts a Military Grid Reference System text to a latitude and longitude on the
WGS-84 ellipsoid: the centre of the square that the text names. Spaces and case do
not matter.

Stack: `text` ▶ `φ` `λ`.

The square of 1 m that holds 45.5°N, 73.6°W, whose centre is about 0.2 m from it:

```rpl
"18T XR 09377 39449" ⓁMGRS→LL 2 →LIST
@ Expecting { 45.49999 80594 ° -73.59999 80952 ° }
```

The same text in any form, in Sydney:

```rpl
"56h lh 3441651925" ⓁMGRS→LL 2 →LIST
@ Expecting { -33.85999 87578 ° 151.21000 1172 ° }
```


## WMM

The World Magnetic Model 2025 of NOAA and the British Geological Survey: the
magnetic declination at a point, which turns a true course into a magnetic course,
and the direction and strength of the Earth's magnetic field.

Stack: `φ` `λ` `h` `t` ▶ `D` `I` `H` `F`. The inputs are the latitude, the longitude
(East positive), the height above the WGS-84 ellipsoid, and the date as a decimal
year, for instance 2027.5 for the middle of 2027. The results are the declination
(East positive), the inclination (down positive), the horizontal intensity and the
total intensity.

The model is valid from 2025 to 2030; outside these years, the function stops with
an error. It describes the field of the Earth's core and upper crust only, so local
anomalies and magnetic disturbances can move a compass further. According to NOAA,
compasses become unreliable where the horizontal intensity falls below 2 000 nT, close
to the magnetic poles, and call for caution below 6 000 nT.

The function sums 90 terms of a spherical harmonic series, up to degree 12.

Two test values of NOAA: at 80°N, 0°E, at sea level in 2025, it gives D = 1.28°,
I = 83.21°, H = 6 523.2 nT, F = 55 178.5 nT:

```rpl
80 0 0 2025 ⓁWMM 4 →LIST
@ Expecting { 1.28148 21161 3 ° 83.21057 95425 ° 6 523.23096 913 nT 55 178.45461 93 nT }
```

At 80°S, 120°W, 100 km high in 2027.5, it gives D = 67.93°, I = -72.10°,
H = 15 927.0 nT, F = 51 825.7 nT:

```rpl
-80 240 100_km 2027.5 ⓁWMM 4 →LIST
@ Expecting { 67.93164 32316 ° -72.10227 82777 ° 15 927.00798 6 nT 51 825.69071 72 nT }
```


## TrackLen

The length of a track on the WGS-84 ellipsoid: the sum of the geodesic distances
between successive points, computed with the method of T. Vincenty.

Stack: `points` ▶ `s`. Each point of the list is `{ φ λ }` or, as in a route,
`{ "name" φ λ }`. At least two points are needed.

From Montréal to Québec, Saguenay and Sept-Îles; GeographicLib gives 806 199.048 m:

```rpl
{ { 45.5 -73.6 } { 46.8 -71.2 } { 48.4 -71.05 } { 50.2 -66.4 } } ⓁTrackLen
@ Expecting 806 199.04823 4 m
```

A route of the Route subsection can be used directly, for instance `Route ⓁTrackLen`.


## PolyArea

The area of a polygon on the WGS-84 ellipsoid. The polygon closes by itself, and its
corners may be listed in either direction.

Stack: `corners` ▶ `A`. Each corner of the list is `{ φ λ }` or `{ "name" φ λ }`. At
least three corners are needed, and the edges must not cross each other.

The method is the spherical excess on the sphere of the same area, using authalic
latitudes (J. P. Snyder, *Map Projections: A Working Manual*, 1987). Its edges are great
circles of that sphere, not geodesics of the ellipsoid. Against GeographicLib, the
difference is too small to see for a field of 1 km, about 0.002 % for a square of 10°
or a province, 0.04 % for a square of 60°, and 0.16 % for a triangle spanning an ocean.

A field of about 1 km on a side; GeographicLib gives 1 016 253.002 m²:

```rpl
{ { 45.5 -73.6 } { 45.5 -73.587 } { 45.509 -73.587 } { 45.509 -73.6 } } ⓁPolyArea
@ Expecting 1 016 253.00234 m↑2
```

A square of 10° north of the equator; GeographicLib gives 1.227 877 × 10¹² m²:

```rpl
{ { 0 0 } { 0 10 } { 10 10 } { 10 0 } } ⓁPolyArea
@ Expecting 1.22785 73630 7⁳¹² m↑2
```


## LLH→ENU

The position of a point in the local East, North, Up frame of an origin, both given
by latitude, longitude and height on the WGS-84 ellipsoid. It suits short distances:
`U` is measured along the vertical of the origin, not of the point.

Stack: `φ0` `λ0` `h0` `φ` `λ` `h` ▶ `E` `N` `U`.

The method is the rotation of the difference of Earth-centred coordinates, as in
IOGP Guidance Note 7-2.

A point about 1.4 km from Montréal, checked with a separate computation:

```rpl
45.5 -73.6 30 45.51 -73.59 130 ⓁLLH→ENU 3 →LIST
@ Expecting { 781.45816 607 m 1 111.48778 824 m 99.85521 00381 m }
```


## ENU→LLH

The latitude, longitude and height of a point given in the local East, North, Up frame
of an origin on the WGS-84 ellipsoid.

Stack: `φ0` `λ0` `h0` `E` `N` `U` ▶ `φ` `λ` `h`.

The previous example in reverse:

```rpl
45.5 -73.6 30 781.4581660693741 1111.4877882355224 99.85521003776165 ⓁENU→LLH 3 →LIST
@ Expecting { 45.51 ° -73.59 ° 130. m }
```


## UTC→GPS

Converts a UTC date and time to GPS week and time of week. GPS time started on 6 January
1980 at midnight UTC, and it has no leap seconds: it has been 18 seconds ahead of UTC
since 1 January 2017.

Stack: `date` `time` ▶ `week` `tow`, with the date as `YYYYMMDD` and the time as
`HH.MMSS`. The week number has no rollover, and `tow` is the time since the start of
the week, on Sunday.

Before 2017 the offset was smaller, so the function stops with an error. A leap second
added later would require a new offset here.

```rpl
20261102 12.00 ⓁUTC→GPS 2 →LIST
@ Expecting { 2 443 129 618 s }
```


## GPS→UTC

Converts a GPS week and time of week to a UTC date and time.

Stack: `week` `tow` ▶ `date` `time`. The range is that of UTC→GPS. The offset of 18 s
may fall on the previous day:

```rpl
2443 129618 ⓁGPS→UTC 2 →LIST
@ Expecting { Mon 2/Nov/2026 12:00:00 }
```

```rpl
2443 5_s ⓁGPS→UTC 2 →LIST
@ Expecting { Sat 31/Oct/2026 23:59:47 }
```


## LL→DMS

A latitude and longitude as a text in degrees, minutes and seconds, to 0.01″, with the
letters of the hemispheres.

Stack: `φ` `λ` ▶ `text`.

```rpl
-33.86 151.21 ⓁLL→DMS
@ Expecting "33°51′36.00″S 151°12′36.00″E"
```

Rounding carries over to the minutes and the degrees:

```rpl
59.999999 0.0001 ⓁLL→DMS
@ Expecting "60°00′00.00″N 0°00′00.36″E"
```


## LL→DDM

A latitude and longitude as a text in degrees and decimal minutes, to 0.001′, the form
that many GPS receivers show.

Stack: `φ` `λ` ▶ `text`.

```rpl
45.5 -73.6 ⓁLL→DDM
@ Expecting "45°30.000′N 73°36.000′W"
```


## RouteLibrary

The Route subsection keeps a route — a list of waypoints — and follows it
while travelling.

![Route and waypoints](img/RouteWaypoints.bmp)

* The route is the variable `Route` of the current directory: a list with one
  `{ "name" φ λ }` list per waypoint. It can be viewed and edited like any
  list.
* The active leg is the variable `RouteLeg`: leg 1 goes from the first to the
  second waypoint.

To build a route: [WPAdd](#wpadd) adds a waypoint at the end,
[WPIns](#wpins) inserts one, for instance to divert, and [WPDel](#wpdel)
removes one. [WPLegs](#wplegs) lists the legs with their distance and course.

On the way: [WPNext](#wpnext) gives, from the present position, the bearing,
distance and time to the next waypoint, and how far off the route one is; it
moves to the next leg by itself once a waypoint is passed.
[WPDirect](#wpdirect) goes straight from the present position to a chosen
waypoint.

Distances and bearings are computed on the WGS-84 ellipsoid with
[VincentyInv](#vincentyinv).


## WPAdd

Adds a waypoint at the end of the route, and creates the route if needed.

Stack: `"name"` `φ` `λ` ▶ nothing; the waypoint is added to `Route`.

The examples of this subsection first purge any previous route, build a
route from Los Angeles to New York through Albuquerque, then show the names of
its waypoints, with `Route « 1 GET » MAP`:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
Route « 1 GET » MAP
@ Expecting { "LAX" "ABQ" "JFK" }
```

See the figure in [Route](#routelibrary): waypoints, legs and the active leg.


## WPIns

Inserts a waypoint into the route: it becomes waypoint `n`, and the following
ones move down by one. This is how to divert, or to add a turning point. The
active leg follows the waypoints it pointed to.

Stack: `"name"` `φ` `λ` `n` ▶ nothing.

A stop at Denver, inserted between Albuquerque and New York:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
"DEN" 39.8561 -104.6737 3 ⓁWPIns
Route « 1 GET » MAP
@ Expecting { "LAX" "ABQ" "DEN" "JFK" }
```

See the figure in [Route](#routelibrary): waypoints, legs and the active leg.


## WPDel

Removes waypoint `n` from the route. The active leg follows the waypoints it
pointed to.

Stack: `n` ▶ nothing.

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
2 ⓁWPDel
Route « 1 GET » MAP
@ Expecting { "LAX" "JFK" }
```

See the figure in [Route](#routelibrary): waypoints, legs and the active leg.


## WPLegs

Lists the legs of the route: for each leg, `"A→B"`, its distance in nautical
miles and its initial true course; then the total distance.

Stack: ▶ `{ legs }` `Total`.

The example shows the total, then the distance of each leg:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
ⓁWPLegs SWAP « 2 GET » MAP LIST→ DROP 3 →LIST
@ Expecting { Total:2 174.82710 563 nmi 588.50367 4436 nmi 1 586.32343 12 nmi }
```

See the figure in [Route](#routelibrary): waypoints, legs and the active leg.


## WPNext

Where is the next waypoint, and how far off the route am I?

![Route and waypoints](img/RouteWaypoints.bmp)

Stack: `φ` `λ` `GS` ▶ `To` `Brg` `Dist` `XTD` `ETE` `ETA`, from the present
position and the ground speed (knots if a plain number; 0 to skip `ETE` and
`ETA`):

* `To`: the next waypoint, the end of the active leg;
* `Brg` and `Dist`: its true bearing and its distance in nautical miles;
* `XTD`: the cross-track distance from the active leg, positive when right of
  it;
* `ETE`: the Estimated Time En route, the time left to reach it at the
  ground speed, in hours, minutes and seconds;
* `ETA`: the Estimated Time of Arrival, the time by the clock of the
  calculator when it will be reached.

When the present position is abeam or past the end of the active leg, the
next leg becomes active, and the results refer to it.

Over the Mojave desert, at 450 kt, on the way from Los Angeles to Albuquerque.
The examples drop the `ETA`, which depends on the time they are run:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
34.3 -114.0 450 ⓁWPNext DROP 5 →LIST
@ Expecting { To:"ABQ" Brg:80.99381 77706 ° Dist:368.36060 6014 nmi XTD:11.28267 43886 nmi ETE:0:49:07 }
```

East of Albuquerque, the first leg is behind: the second leg, to New York,
becomes active, and `RouteLeg`, added at the end, is 2:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
35.3 -104.0 450 ⓁWPNext DROP 5 →LIST RouteLeg +
@ Expecting { To:"JFK" Brg:68.26395 64658 ° Dist:1 461.44173 207 nmi XTD:32.14230 0708 nmi ETE:3:14:52 2 }
```


## WPDirect

"Direct to": go straight from the present position to waypoint `n` of the
route. The present position is inserted before waypoint `n` under the name
`"DCT"`, and the leg from it to waypoint `n` becomes the active leg.

Stack: `φ` `λ` `n` ▶ nothing.

Over Los Angeles, cleared direct to New York; the route now goes through
`"DCT"`, and the active leg is the third, from `"DCT"` to New York:

```rpl
{ Route RouteLeg } PURGE
"LAX" 33.9425 -118.4081 ⓁWPAdd
"ABQ" 35.0402 -106.6090 ⓁWPAdd
"JFK" 40.6398 -73.7789 ⓁWPAdd
34.0 -118.0 3 ⓁWPDirect
Route « 1 GET » MAP RouteLeg +
@ Expecting { "LAX" "ABQ" "DCT" "JFK" 3 }
```

See the figure in [Route](#routelibrary): waypoints, legs and the active leg.


