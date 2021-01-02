# pat's rack modules
a set of modules i have created for vcv rack :^)

## installing

for windows:
  place the *contents* of dist/pat-VERSION-win.zip into your plugins-v1 folder (located in Documents/Rack/plugins-v1)

## modules

### chance
takes a gate input, and has a chance to pass the input to any of its four outputs. the probability is independant, and can be multiplied by an incoming cv signal (0 to 10 volts).

### renick
a simple l-system module. it has four symbols, a,b,c,d, each of which have a corresponding duration in ms. there is also a user defined rule attached to each symbol. 

a basic example of an l-system would be:
  - begin with a word, i.e acdcab
  - and a set of rules, i.e, a -> bc, b -> dda, c -> cc, d -> db
  - loop over the word, a then c then d then c then a then b
  - iterate over the word, replacing each symbol according to that symbol's rule, i.e, our word would become bc cc db cc bc dda
  - continue until you reach ultimate contentment
  
each symbol is then interpreted as an ammount of time to wait before triggering an output.

### hold me
a sample-hold and/or range-mapper. it takes an input signal, and an input range [min,max], and maps it to an output range [start,end]. optionally, you can enable "gate?" to make the output wait for a gate input before updating its value.

### polyamory
takes 4 inputs (or more, if you put a poly cable into input a), and distrubutes them evenly over the range [0,1]. it then adjusts the volume of each input according to its distance to the center and width, and outputs the sum. the mul parameter allows for additional volume control.

### timothy
a clock. has outputs for 1,2,4,8, and 16 beats. also has a speed multiplier toggle, so you can multiply the speed by either 1/4, 1/2, 2 or 4. the bpm has a cv output as well, so you can sync up other clocks?

### snap
a perhaps useless clock subdivider. takes a bpm and a number of beats, and divides the duration by the "div". i.e, if you had 120 bpm, with 4 beats, and a div of 3, it would trigger 3 times over the next 4 beats.
