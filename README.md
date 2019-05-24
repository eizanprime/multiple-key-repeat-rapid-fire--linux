# multiple-key-repeat linux

Both the linux kernel kbd driver and the key repeat of xorg in xset dont support multiple key repeat (rapid fire), meaning that when pressing "a" and "o" simultaneously, you will get "aoooooooooooooooooooo" instead of the expected "aoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoaoao".

After trying multiple approaches, it finally came to this radical approach of piping the input of the keyboard in /dev/input/by-id/*your keyboard*, doing complicated thing to implement the behavior, and sending the events to /dev/uinput. 

The program accept command line arguments being the path to the keyboard in /dev/input/by-id/, -r the rate of repeat per seconds, and -d the initial delay in miliseconds. 

You can compile the program with gcc ./multiple-key-repeat.c -o executable-file-name -lpthread

I part of the code are from stack overflow (I will link the topic here later) 
