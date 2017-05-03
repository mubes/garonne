"
                                       ++++++++++++++++++
                                  +++++++++++++++++++++++++++++
                              +++++++                      +++++++++
                          +++++++                               +++++++++++++
         ++++++++++++++++++++                                         ++++++++++
    +++++++++++++++++++++                                                     +++
   +++++                                                                       +++
  +++         ######### ######### ########  #########  #########   +++++++      ++
  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++


#Introduction
The Garonne project is a simple infrastructure for autonomous and ADAS model vehicles. It
has been run on numerous embedded and application platforms. The system consists of 
a Low Level Board (LLB) which is a real time embedded plaform (optionally multi-core)
and a High Level Board (HLB) which performs the 'reasoning' behind the actions the 
system takes.  Many different platforms have been used for the HLB and the LLB including
Tegra TK1 and full blown PCs. 

Its designed to be cheap to build, modify and crash....but to be a reasonably powerful and extensible platform at the same time.

In the current implementation, for ease of access, the High Level Board minially a 
Raspberry Pi and the Low Level Board is a NXP LPCXpresso LPC4367 204MHz CORTEX-M4 + 2x
CORTEX M0 which provides a significant amount of real time computing power. The HLB
runs Linux, programmed in Python, and the LLB is based on FreeRTOS and is programmed
in C.

The project was orignally created by Technolution BV. It was first demonstrated at the 
ITS World Congress in Bordeaux in 2016 (Hence, the codename 'Garonne', the river that 
Bordeaux lies on) based on bespoke hardware. It has been ported to COTS hardware by
Technolution and is now released for open use.

#Getting Started
Generally speaking, a Garonne system will consist of at least one vehicle, a control platform (consisting of a Low Level Board (LLB) and a High Level Board (HLB) and a command platform, which is generally remote. All of this runs on a physical vehicle...so, let's get started;

##Physical Vehicle
Generally speaking, you can use most remote controlled cars for the physical vehicle, the requirements are not very onerous. Once you've got it all built up, the vehicle will look something like this;

![Vehicle](https://www.dropbox.com/s/fn6lnabuu2fop0k/IMG_20170503_131325.jpg?dl=0)

The shopping list looks like the following;

* ***M.Rage chassis, assembled***.  [Hobbyking](https://hobbyking.com/en_us/bsr-racing-m-rage-1-10-4wd-m-chassis-un-assembled-kit.html). The assembled bit is an exercise for the reader.

* ***Steering Servo***...just a bog standard analogue servo will be fine. Something like [this](https://hobbyking.com/en_us/turnigytm-tgy-9025mg-mg-servo-1-8kg-0-09sec-11g.html).

* ***Pololu motor***, mounted in chassis, with drive pinion. We use the 4.4:1 variant which is expensive, but comes with itw own encoder and they're available from distributors around the world. [Pololu item number 2270] (https://www.pololu.com/product/2270).

* ***Drive Pinion***. The most specific bit of the whole setup. These are 31 tooth 48P pinions with 5mm bore. We get ours from neil at rwracing.co.uk. Take a guess at his email address.

* ***Power Switch***. You can use anything for this really, but the plastics have a 13x19.5 hole cut to take a snap-in switch. The ones we have are marked KCD1-104 and work fine. Available for cheap from the usual outlets.

* ***Mounting plastic for the electronics***. No way out of this one, you've got to make them, or talk very nicely to me to see if we've got any spare, we often have, but shipping outside of local territory tends to end up more expensive than making new ones.  The drawings to make them from are in the git repository, and [Elecrow] (https://www.elecrow.com/) will make you more than you'll ever need in 3mm acrylic for very cheap.

* ***LPC4367 LPCXpresso board***. These are available from [Farnell] (http://uk.farnell.com/nxp/om13088/dev-board-32bit-dual-core-mcu/dp/2519578) and elsewhere.  Again, not cheap, but you're getting a CORTEX-M4 and two CORTEX-M0 CPUs here, complete with development environment and debugger.

* ***MPU9250 9D sensor board***. Search for '9DOF MPU9250' or similar on eBay. It's just a chip on a board so it's difficult for them to screw it up too badly. Make sure you end up with a 9250 board and not a 65 series board though....they're largely identical but the one with MPU65 written on the top doesn't have the magnetometer built in.

* ***Raspberry Pi 3***...'nuff said. The plastics also support the little mini ones, and the PiW has wifi on now too. Which one you choose is dependent on how much processing you want to do on the vehicle. We've used all sorts for this job from a Pi to a TEGRA K1 and a Parallella. You can connect to the LLB via TTL-serial or USB, so there's flexibility here.

* ***VHN2SP30 motor driver***. Again, this is an eBay job.  You only need the single one, not the dual that is set up as an Arduino shield.

* ***5V power supply***. There are 1001 of these. We use the little MP1584 modules set to 5V with a dab of nail varnish on the pot afterwards. Use whatever you prefer but *beware* there are many stories of fake modules coming out of China. They usually work fine but I wouldn't trust them anywhere near their stated current limits!

In addition to this lot you will need some male to female and female to female 10cm and 20cm Dupont jumpers, and some nylon m3 standoff pillars. In due course I'll produce a whole wiring digram with colours and everything.