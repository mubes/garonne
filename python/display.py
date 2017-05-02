#                                       ++++++++++++++++++
#                                  +++++++++++++++++++++++++++++
#                             +++++++                      +++++++++
#                          +++++++                               +++++++++++++
#         ++++++++++++++++++++                                         ++++++++++
#    +++++++++++++++++++++                                                     +++
#   +++++                                                                       +++
#  +++         ######### ######### ########  #########  #########   +++++++      ++
#  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
#  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
#   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
#    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++
#
# Display Module
# ==============
#
# Simple curses based diagnostic interface for on-vehicle use.
#
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

import curses

class Display:
    def __init__(self,stdscr):
        self.COL_DIST=1
        self.COL_DISTE=2
        self.COL_DISTT=3
        self.COL_NORM=4
        self.COL_REVS=5
        self.COL_MOTORSPEED=6
        self.COL_LOG=7        
        self.COL_0=8
        self.COL_1=9
        self.COL_2=10
        self.COL_3=11
        self.COL_4=12
        self.COL_5=13
        self.COL_6=14
        self.COL_7=15
        self.COL_STEERANGLE=16
        self.COL_MOTORSPEEDE=17
        self.screen=stdscr
        self.screen.clear()
        self.SY=24
        self.SX=80
        self.screen.scrollok(True)
        curses.curs_set(False)
        self.screen.setscrreg(13,self.SY-1)
        curses.init_pair(self.COL_DIST,curses.COLOR_BLACK,curses.COLOR_BLUE)
        curses.init_pair(self.COL_DISTE,curses.COLOR_BLACK,curses.COLOR_RED)
        curses.init_pair(self.COL_DISTT,curses.COLOR_BLUE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_NORM,curses.COLOR_YELLOW,curses.COLOR_BLACK)
        curses.init_pair(self.COL_REVS,curses.COLOR_BLUE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_LOG,curses.COLOR_GREEN,curses.COLOR_BLACK)
        curses.init_pair(self.COL_MOTORSPEED,curses.COLOR_BLUE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_MOTORSPEEDE,curses.COLOR_RED,curses.COLOR_BLACK)

        curses.init_pair(self.COL_0,curses.COLOR_WHITE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_1,curses.COLOR_RED,curses.COLOR_BLACK)
        curses.init_pair(self.COL_2,curses.COLOR_GREEN,curses.COLOR_BLACK)
        curses.init_pair(self.COL_3,curses.COLOR_YELLOW,curses.COLOR_BLACK)
        curses.init_pair(self.COL_4,curses.COLOR_BLUE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_5,curses.COLOR_MAGENTA,curses.COLOR_BLACK)
        curses.init_pair(self.COL_6,curses.COLOR_CYAN,curses.COLOR_BLACK)
        curses.init_pair(self.COL_7,curses.COLOR_WHITE,curses.COLOR_BLACK)
        curses.init_pair(self.COL_STEERANGLE,curses.COLOR_GREEN,curses.COLOR_BLACK)        

    def addLog(self, s):
        """
        Send message to screen and scroll appropriately.
        """
        self.screen.scroll()
        self.screen.addstr(self.SY-1,0,s,curses.color_pair(self.COL_LOG))
        self.screen.refresh()
        
    def doUpdate(self, t, c):
        """
        Update the screen with new information by repainting.

        This is grubby with a lot of magic numbers due to the nature of 
        screen working. A nicer library could be used, but this is only
        really intended for development use anyway.
        """
        
        # Ultrasonics ==============================================
        self.screen.addstr(1,0,"-"*self.SX,curses.color_pair(self.COL_NORM))
        self.screen.addstr(1,int(self.SX/2)-1,"[Car]",curses.color_pair(self.COL_NORM))

        if ("FrontUltrasonic" in t):
            self.screen.addstr(0,15,str(t["FrontUltrasonic"]["dist"])+" mm       ",curses.color_pair(self.COL_DISTT))
            colour=self.COL_DIST
            if (("EStop" in t) and (t["EStop"]["front"])):
                colour=self.COL_DISTE
            if ((t["FrontUltrasonic"]["dist"])<1000):
                inDist=1+int((((self.SX/2)-2)/1000)*(1000-t["FrontUltrasonic"]["dist"]))
                self.screen.addstr(1,0," "*inDist,curses.color_pair(colour))
            if (colour==self.COL_DISTE):
                self.screen.addstr(1,15,"<<F-ESTOP>>",curses.color_pair(colour))

        if ("RearUltrasonic" in t):
            self.screen.addstr(0,self.SX-15,str(t["RearUltrasonic"]["dist"])+" mm       ",curses.color_pair(self.COL_DISTT))
            colour=self.COL_DIST
            if (("EStop" in t) and (t["EStop"]["rear"])):
                colour=self.COL_DISTE
            if ((t["RearUltrasonic"]["dist"])<1000):
                inDist=1+int((((self.SX/2)-5)/1000)*(1000-t["RearUltrasonic"]["dist"]))
                self.screen.addstr(1,self.SX-inDist," "*inDist,curses.color_pair(colour))
            if (colour==self.COL_DISTE):
                self.screen.addstr(1,int(self.SX/2)+15,"<<R-ESTOP>>",curses.color_pair(colour))

        # Revs =====================================================
        self.screen.addstr(3,2,"WheelTicks:",curses.color_pair(self.COL_NORM))
        if ("revs" in t):
            self.screen.addstr(3,15,str(t["revs"]["ticks"])+"         ",curses.color_pair(self.COL_REVS)) 

        # Motor Speed ===============================================
        self.screen.addstr(4,2," Motor Spd:",curses.color_pair(self.COL_NORM))
        colour=self.COL_MOTORSPEED
        if ("motor" in t):
            if (("EStop" in t) and
                (((t["motor"]["speed"]<0) and (t["EStop"]["rear"])) or
                 ((t["motor"]["speed"]>0) and (t["EStop"]["front"])))):

                colour=self.COL_MOTORSPEEDE
            self.screen.addstr(4,15,str(t["motor"]["speed"])+"%      ",curses.color_pair(colour))

        # Battery  ===================================================
        self.screen.addstr(6,2,"   Battery:",curses.color_pair(self.COL_NORM))
        self.screen.addstr(7,2,"      Temp:",curses.color_pair(self.COL_NORM))
        if ("batt" in t):
            self.screen.addstr(6,15,str(t["batt"]["v"]/1000)+"V   ",curses.color_pair(self.COL_REVS))
            self.screen.addstr(7,15,str(int(t["batt"]["temp"]/100))+"."+str(int(t["batt"]["temp"]/10)%10)+"°C  ",curses.color_pair(self.COL_REVS))             

        # 9D  ========================================================
        self.screen.addstr(3,30,"        Acc       Gyr                    Mag",curses.color_pair(self.COL_NORM))
        self.screen.addstr(4,30,"+---+---------+---------+         +---+-------+",curses.color_pair(self.COL_NORM))        
        self.screen.addstr(5,30,"| X |         |         |         | 0 |       |",curses.color_pair(self.COL_NORM))
        self.screen.addstr(6,30,"| Y |         |         |         | 1 |       |",curses.color_pair(self.COL_NORM))
        self.screen.addstr(7,30,"| Z |         |         |         | 2 |       |",curses.color_pair(self.COL_NORM))
        self.screen.addstr(8,30,"+---+---------+---------+         +---+-------+",curses.color_pair(self.COL_NORM))        
        if ("9d" in t):
            for i in range(0,3):
                self.screen.addstr(5+i,36,str(t["9d"]["acc"][i]),curses.color_pair(self.COL_REVS))
                self.screen.addstr(5+i,46,str(t["9d"]["gyr"][i]),curses.color_pair(self.COL_REVS))
                self.screen.addstr(5+i,70,str(t["9d"]["mag"][i]),curses.color_pair(self.COL_REVS))

        # Turrets  ====================================================
        for i in range(0,8):
            self.screen.addstr(9,7+9*i,str(i),curses.color_pair(self.COL_0+i))
            if (("Turret" in t) and (i in t["Turret"]) and (t["timestamp"]-t["Turret"][i]["ts"]<2000)):
                self.screen.addstr(10,5+9*i,str(t["Turret"][i]["value"])+"/"+str(t["Turret"][i]["count"])+"   ",curses.color_pair(self.COL_0+i))
            else:
                self.screen.addstr(10,5+9*i,"--/--",curses.color_pair(self.COL_0+i))
        # Steer Angle =====================================================
        self.screen.addstr(12,1,"|1000|"+"."*(int(self.SX/2)-8)+"|500|"+"."*(int(self.SX/2)-8)+"|0|",curses.color_pair(self.COL_NORM))
        if ("steer" in t):
            self.screen.addstr(11,0," "*self.SX)
            inDist=int(((self.SX-4)/1000)*(1000-t["steer"]["angle"]))
            self.screen.addstr(11,inDist+1,str(t["steer"]["angle"]),curses.color_pair(self.COL_STEERANGLE))
            if (t["steer"]["angle"]<10):
                inDist-=1
            self.screen.addstr(12,inDist+2,"|",curses.color_pair(self.COL_STEERANGLE))
        self.screen.refresh()

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

