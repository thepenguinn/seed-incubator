// $fn = 20;

TotalWidth  = 50 * 10;
TotalHeight = 50 * 10;
TotalLength = 80 * 10;

BaseWidth  = TotalWidth;
BaseLength = TotalLength;
BaseThickness = 5;

ThermSideTruncated = 1 * 10;

ThermTotalWidth = BaseWidth - 2 * ThermSideTruncated;
ThermTotalLength = 15 * 10;
ThermTotalHeight = 15 * 10;

use <inc_header.scad>
use <thermal_module.scad>

// just a marker
sphere(4);
translate([10, 0, 0]) sphere(1);

thermal_module(ThermTotalWidth);
