$fn=64;
rlength=100;
length=120;
backlength=130;
width=10;
height=20;

frontcutdepth=2;
frontwidth=19;
frontDepth=13;
hcentres=12.5;
hdep=7.25;

topcutdepth=4;
topcutwidth=18;

postDia=6.8;
screwDia=3.1;
postDist=77;

backpostDist=90;
rpostDist=89.5;
fixrecess=9;

radioWidth=7;
radioDepth=12;
radioLength=14;
radioHold=radioLength+12;
radioHole=4;

module front()
{
difference()
{
    // Top block
    union()
    {
        cube([width,length,height]); 
        translate([3,25,height-0.01]) cube([width-5,length-50,35]);
    }

    // Slopes for upper panel
    translate([0,17,height+6]) rotate([-10,0,0]) cube([width,8,70]);
    translate([0,length-25.01,height+6]) rotate([10,0,0]) cube([width,8,70]);

    
    // LED holes
        translate([-3,30,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,20,10]) rotate([0,90,0]) cylinder(r=1,h=8);
        translate([-3,10,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,length/2,10]) rotate([0,90,0]) cylinder(r=1,h=8);    
        translate([-3,length-30,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,length-20,10]) rotate([0,90,0]) cylinder(r=1,h=8);
        translate([-3,length-10,10]) rotate([0,90,0]) cylinder(r=5,h=15);
    
    
    
    // Uptrasonic holes
   translate([0,length/2-13,height+12]) rotate([0,90,0]) cylinder(r=9,h=12);
   translate([0,length/2+13,height+12]) rotate([0,90,0]) cylinder(r=9,h=12);
    translate([1,length/2-8,height+15]) cube([5,16,8]);    
   
   // Camera
   translate([0,length/2,height+42]) rotate([0,90,0]) cylinder(r=15,h=12);    
    translate([0,length/2+13.5,height+27]) rotate([0,90,0]) cylinder(r=2,h=12);
    translate([0,length/2-13.5,height+27]) rotate([0,90,0]) cylinder(r=2,h=12);
    
    // Front cutout for laser
    translate([width-frontcutdepth+0.01,(length-frontwidth)/2,height-frontDepth]) cube([frontcutdepth,frontwidth,frontDepth+1.05]);
   
   // Holes for moutning VL laser 
    translate([width,(length-hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    translate([width,(length+hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
 
        // Top recess for Dupont connector
  translate([-0.01,(length-topcutwidth)/2,height-topcutdepth]) cube([width,topcutwidth,topcutdepth+0.05]);
    
    
    // Mounting screws
    translate([width/2,(length-postDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
    translate([width/2,length-(length-postDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
    // Remove top when there's no camera mount needed
    translate([0,0,height+25]) cube([10,100,height+25]);
    
}    
}

module back()

{
difference()
{
    // Top block
    union()
    {
        cube([width,backlength,height]); 
        translate([3,25,height-0.01]) cube([width-5,backlength-50,27]);
    }
    
        // Slopes for upper panel
    translate([0,17,height+6]) rotate([-10,0,0]) cube([width,8,70]);
    translate([0,backlength-25.01,height+6]) rotate([10,0,0]) cube([width,8,70]);

    // Uptrasonic holes
   translate([0,backlength/2-13,height+12]) rotate([0,90,0]) cylinder(r=9,h=12);
   translate([0,backlength/2+13,height+12]) rotate([0,90,0]) cylinder(r=9,h=12);
    translate([1,backlength/2-8,height+15]) cube([5,16,8]);  

    
    // LED holes
        translate([-3,30,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,20,10]) rotate([0,90,0]) cylinder(r=1,h=8);
        translate([-3,10,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,backlength/2,10]) rotate([0,90,0]) cylinder(r=1,h=8);    
        translate([-3,backlength-30,10]) rotate([0,90,0]) cylinder(r=5,h=15);
        translate([-1,backlength-20,10]) rotate([0,90,0]) cylinder(r=1,h=8);
        translate([-3,backlength-10,10]) rotate([0,90,0]) cylinder(r=5,h=15);
       
    // Front cutout for laser
    translate([width-frontcutdepth+0.01,(backlength-frontwidth)/2,height-frontDepth]) cube([frontcutdepth,frontwidth,frontDepth+1.05]);
   
   // Holes for moutning VL laser 
    translate([width,(backlength-hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
    translate([width,(backlength+hcentres)/2,height-topcutdepth-hdep]) rotate([0,-90,0]) cylinder(r=1,h=8);
 
        // Top recess for Dupont connector
  translate([-0.01,(backlength-topcutwidth)/2,height-topcutdepth]) cube([width,topcutwidth,topcutdepth+0.05]);
    
    
    // Mounting screws
    translate([width/2,(backlength-backpostDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
    translate([width/2,backlength-(backlength-backpostDist)/2,-0.01])  
    {
        cylinder(r=postDia/2,h=fixrecess);
        cylinder(r=screwDia/2,h=height+0.05);
    }
    
}    
}


//rotate([0,0,180]) translate([0,-50-(length/2),0]) 
back();
//translate([2,0,0]) back();
