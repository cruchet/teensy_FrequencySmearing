# teensy_FrequencySmearing
# Idea and background
One of the consequences of hearing impairment, is a decrease in the ability to discriminate frequency. This can be modeled by smearing the audio spectrum. The idea and goal of this project is to implement a smearing algorithm on an embedded platform to process audio in real-time. This could then be used as ademonstrator in different events where people could wear headphones and experiencethis aspect of hearing impairment and the deficit in communication that comes with it.In practice, the algorithm was first implemented in MATLAB to test it off-line with different parameters. Secondly, the algorithm was implemented on a microcontroller-based platform (TEENSY 3.6) which integrate a DSP co-processor with which very complete libraries can be used.
For a full and more complete documentation, see the #PDF MANUAL#

# Hardware used
Platform used is a [TEENSY 3.6 board](https://www.pjrc.com/store/teensy36.html), that uses an ARM Cortex-M4F is a 32 bit processor, clocked at 180MHz. 
It is combined to the [TEENSY audio shield](https://www.pjrc.com/store/teensy3\_audio.html) is used to provide audio interface in a high 16 bits quality.
It is therefore based on the [PJRC Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html) and a modified version of the [CSIS DSP library](https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html).

# Matlab code
