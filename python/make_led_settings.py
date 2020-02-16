import tkinter as tk

grid_size = 8

class LEDButton(tk.Canvas):
    def __init__(self, parent, width, height, color, command=None):
        super().__init__(parent, borderwidth=1,
            highlightthickness=0)
        self.command = command
        self.state = 0
        self.colors = ['white', 'green', 'red']

        self.padding = 4
        self.width = width
        self.height = height
        self.create_oval((self.padding,self.padding,
            self.width+self.padding, self.height+self.padding), outline=color, fill=color)
        (x0,y0,x1,y1)  = self.bbox("all")
        self.width = (x1-x0) + self.padding
        self.height = (y1-y0) + self.padding
        self.configure(width=self.width, height=self.height)
        self.bind("<ButtonPress-1>", self._on_press)

    def _on_press(self, event):
        self.state += 1
        if self.state == len(self.colors) : 
            self.state = 0

        col = self.colors[self.state]

        self.create_oval((self.padding,self.padding,
            self.width, self.height), outline=col, fill=col)

        self.configure()

    def set_color( self, color ) : 
        if color not in self.colors : 
            print ('LEDButton -- ERROR : Color %s is not in my list' %color )
        else : 
            self.state = self.colors.index(color )

            
            self.delete('all')
            self.create_oval((self.padding,self.padding,
                self.width, self.height), outline=color, fill=color)

            self.configure()

    def get_color(self) : 
        return self.colors[self.state]
    def get_state( self) : 
        return self.state
            

class CaptureButton( tk.Button ) : 

    def __init__(self, master ) : 
        super().__init__(master, text='Capture', fg='black')

        self.bind("<ButtonPress-1>", self._on_press)

    def set_buttons( self, buttons ) : 
        self.buttons = buttons

    def set_duration( self, button ) : 
        self.duration = button

    def _on_press( self, event ) : 

        entries = []
        for iy in range(0, grid_size ) : 
            activation = 0x0;
            is_green   = 0x0;
            for ix in range(0, grid_size ) : 
                stat = self.buttons[(ix, iy)].get_state()
                if stat > 0 : 
                    activation |= (1 << (grid_size-1-ix))
                if stat == 1 :  
                    is_green |= (1 << (grid_size-1-ix) )

            entries.append(activation)
            entries.append(is_green)

        out_str = 'display_pattern('
        out_str += (', '.join( ['0x'+format( x, '0x') for x in entries] ))
        out_str += ', %s );' %self.duration.get()

        print (out_str)
                                

class ColorButton( tk.Button ) : 

    def __init__(self, master, color ) : 
        super().__init__(master, text=color, fg='brown')

        self.color = color
        self.width = 50
        self.height = 30

        #self.configure( width=self.width, height=self.height )
        self.bind("<ButtonPress-1>", self._on_press)

    def set_buttons( self, buttons ) : 
        self.buttons = buttons

    def _on_press( self, event ) : 
        for b in self.buttons.values() : 
            b.set_color(self.color)



class TopFrame(tk.Frame):
    def __init__(self, master=None, led_screen=None) : 
        super().__init__(master)
        self.master = master
        self.led_screen = led_screen


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        #self.pack()
        self.create_widgets()
        self.buttons = {}

    def create_widgets(self):
        #self.white_button = tk.Button(self.master, text='white', fg='red' )
        self.white_button = ColorButton(self.master, 'white' )
        self.red_button = ColorButton(self.master, 'red' )
        self.green_button = ColorButton(self.master, 'green' )
        #self.hi_there["text"] = "Hello World\n(click me)"
        #self.hi_there["command"] = self.say_hi
        #self.hi_there.pack(side="top")

        #self.white_button.pack(side='top')

        
        self.white_button.grid(row=0, column=0, columnspan=2 )
        self.red_button.grid(row=0, column=2, columnspan=2 )
        self.green_button.grid(row=0, column=4, columnspan=2 )

        self.buttons = {}
        for ix in range(0, grid_size ) : 
            for iy in range(0, grid_size ) : 
                self.buttons[(ix, iy)] = LEDButton( self.master, 30, 30, 'white' )
                self.buttons[(ix, iy)].grid(row=ix+1, column=iy)

        self.white_button.set_buttons( self.buttons )
        self.red_button.set_buttons( self.buttons )
        self.green_button.set_buttons( self.buttons )

        self.time_lab = tk.Label( self.master, text='Duration')
        self.time_ent = tk.Entry(self.master, width=8 )
        self.time_ent.insert( 0, '100' )

        self.time_lab.grid( row = 10, column=0, columnspan=2 )
        self.time_ent.grid( row = 10, column=2, columnspan=2 )

        self.cap_but = CaptureButton( self.master )
        self.cap_but.grid( row=10, column=4, columnspan=2 )

        self.cap_but.set_buttons( self.buttons )
        self.cap_but.set_duration( self.time_ent )


        #self.test_button.pack()
        #self.test_button2.pack()
        #self.test_can = tk.Canvas(self.master, width=200, height=100 )
        ##self.test_can.create_oval( 0, 20, 0, 20, fill='red' )
        #self.test_can.create_oval(50, 25, 150, 75, fill="blue")
        #self.test_can.pack(side="top")

        #self.quit = tk.Button(self, text="QUIT", fg="red",
        #                      command=self.master.destroy)
        #self.quit.grid(row=0, column=3)
        #self.quit.pack(side="bottom")

    def say_hi(self):
        print("hi there, everyone!")

root = tk.Tk()
app = Application(master=root)

root.mainloop()


    
