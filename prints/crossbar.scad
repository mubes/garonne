$fn=64;
rlength=100;
length=95;
width=10;
height=15;

frontcutdepth=2;
frontwidth=19;
frontDepth=13;
hcentres=12.5;
hdep=7.25;

topcutdepth=2.5;
topcutwidth=14;

postDia=6.5;
screwDia=3.1;
postDist=82.5;
rpostDist=89.5;
fixrecess=4;

radioWidth=7;
radioDepth=12;
radioLength=14;
radioHold=radioLength+12;
radioHole=4;

module front()
{
difference()
{
    union()
    {
        cube([width,length,height]);
        translate([0,(length-topcutwidth)/2-30,height-0.01]) cube([width-3,30,35]);
    }

    translate([-0.01,length/2, height]) rotate([30,0,0]) cube([width,60,60]);
    translate([2,(length)/2-13,height+4]) rotate([0,90,0]) cylinder(r=1,h=12);
    translate([2,(length)/2-13-19,height+30.5]) rotate([0,90,0]) cylinder(r=1,h=12);
    
    translate([width-frontcutdepth+0.01,(length-frontwidth)/2,height-frontDepth]) cube([frontcutdepth,frontwidth,height+0.05]);
    
    translate([width,(length-hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    translate([width,(length+hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    
  translate([-0.01,(length-topcutwidth)/2,height-topcutdepth]) cube([width,topcutwidth,topcutdepth+0.05]);
    
    translate([width/2,(length-postDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=height-fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
    translate([width/2,length-(length-postDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=height-fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
}    
}

module back()

{
 difference()
{
    cube([width,rlength,height]);
    translate([width-frontcutdepth+0.01,(rlength-frontwidth)/2,height-frontDepth]) cube([frontcutdepth,frontwidth,height+0.05]);
    
    translate([width,(rlength-hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    translate([width,(rlength+hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    
  translate([-0.01,(rlength-topcutwidth)/2,height-topcutdepth]) cube([width,topcutwidth,topcutdepth+0.05]);
    
    translate([width/2,(rlength-rpostDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=height-fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
    translate([width/2,rlength-(rlength-rpostDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=height-fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }

    }       
}   


rotate([0,0,180]) translate([0,-50-(length/2),0]) front();
//translate([2,0,0]) back();
