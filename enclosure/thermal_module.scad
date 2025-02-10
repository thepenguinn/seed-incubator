FoamLargeThickness = 4;
FoamSmallThickness = 3;
InsideWidth = 8 * 10;

module copy_mirror(axis) {
    children();
    mirror(axis)
    children();
}

module thermal_g_holder_female(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    difference() {
        square([foamSmallThickness * 4, foamSmallThickness * 2]);
        translate([foamSmallThickness, foamSmallThickness]) {
            square([foamSmallThickness * 2, foamSmallThickness]);
        }
        translate([foamSmallThickness * 2, 0]) {
            square([foamSmallThickness, foamSmallThickness]);
        }
    }

}

module thermal_edge_core_plane(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    difference() {
        square([foamLargeThickness * 4, foamLargeThickness * 4]);
        translate([foamLargeThickness * 3, 0]) square([foamLargeThickness, foamLargeThickness * 2]);
        translate([0, foamLargeThickness * 3]) square([foamLargeThickness * 2, foamLargeThickness]);
    }

}

module thermal_edge_top_front_plane(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    translate([InsideWidth / 2, InsideWidth / 2]) {
        thermal_edge_core_plane();
        // bottom
        translate([0, -foamSmallThickness * 2]) {
            thermal_g_holder_female();
        }
        // left
        translate([-foamSmallThickness * 2, 0]) {
            rotate([0, 0, -90]) rotate([0, 180, 0]) thermal_g_holder_female();
        }
        // right
        translate([foamLargeThickness * 3 + foamSmallThickness * 2, -foamLargeThickness]) {
            rotate([0, 0, 90]) thermal_g_holder_female();
        }
    }

}

module thermal_edge_top_back_plane(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    translate([-InsideWidth / 2, InsideWidth / 2]) {
        rotate([0, 0, 90]) {
            thermal_edge_core_plane();
            // right
            translate([0, -foamSmallThickness * 2]) {
                thermal_g_holder_female();
            }
            // bottom
            translate([-foamSmallThickness * 2, 0]) {
                rotate([0, 0, -90]) rotate([0, 180, 0]) thermal_g_holder_female();
            }
        }
    }

}

module thermal_edge_bottom_front_plane(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    translate([insideWidth / 2, -insideWidth / 2]) {

        thermal_edge_core_plane();
        /*translate([-foamSmallThickness * 2, 0]) {*/
        /*    rotate([0, 0, -90]) rotate([0, 180, 0]) thermal_g_holder_female();*/
        /*}*/
        translate([0, foamLargeThickness * 3]) thermal_g_holder_female();
        /*translate([0, foamSmallThickness * 7]) {*/
        /*}*/

        /*rotate([0, 0, -90]) {*/
        /*}*/
    }

}

module thermal_edge_bottom_back_plane(
        foamLargeThickness = FoamLargeThickness,
        foamSmallThickness = FoamSmallThickness,
        insideWidth = InsideWidth
    ) {

    translate([-insideWidth / 2, -insideWidth / 2]) {
        rotate([0, 0, 180]) {
            thermal_edge_core_plane();
            translate([0, -foamSmallThickness * 2]) {
                thermal_g_holder_female();
            }
        }
    }
}

module thermal_module(thermTotalWidth) {

    linear_extrude(thermTotalWidth) {

        thermal_edge_top_front_plane();
        thermal_edge_top_back_plane();
        thermal_edge_bottom_front_plane();
        thermal_edge_bottom_back_plane();

    }

}
