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
* on a sphere: [RadialFix](#radialfix) and [GCPoint](#gcpoint).


## VincentyInv

The inverse geodetic problem: the distance between two points of the WGS-84
ellipsoid, and the azimuth of the shortest route at each end.

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

Stack: `φ1` `λ1` `φ2` `λ2` `f` ▶ `φ` `λ`.

It is not defined for antipodal points, between which every great circle is a
shortest route.

Four tenths of the way from Los Angeles (LAX, 33°57′N, 118°24′W) to New York
(JFK, 40°38′N, 73°47′W); Ed Williams gives 38°40.167′N, 101°37.570′W:

```rpl
33.95 -118.4 40.63333333333333 -73.78333333333333 0.4 ⓁGCPoint 2 →LIST
@ Expecting { 38.66944 7748 ° -101.62616 0313 ° }
```


## RouteLibrary

The Route subsection keeps a route — a list of waypoints — and follows it
while travelling.

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


## WPNext

Where is the next waypoint, and how far off the route am I?

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
