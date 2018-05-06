# teensy_FrequencySmearing
## Idea and background
One of the consequences of hearing impairment, is a decrease in the ability to discriminate frequency. This can be modeled by smearing the audio spectrum. The idea and goal of this project is to implement a smearing algorithm on an embedded platform to process audio in real-time. This could then be used as ademonstrator in different events where people could wear headphones and experiencethis aspect of hearing impairment and the deficit in communication that comes with it.In practice, the algorithm was first implemented in MATLAB to test it off-line with different parameters. Secondly, the algorithm was implemented on a microcontroller-based platform (TEENSY 3.6) which integrate a DSP co-processor with which very complete libraries can be used.
For a full and more complete documentation, see the **PDF MANUAL**

## Hardware used
Platform used is a [TEENSY 3.6 board](https://www.pjrc.com/store/teensy36.html), that uses an ARM Cortex-M4F is a 32 bit processor, clocked at 180MHz. 
It is combined to the [TEENSY audio shield](https://www.pjrc.com/store/teensy3\_audio.html) is used to provide audio interface in a high 16 bits quality.
It is therefore based on the [PJRC Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html) and a modified version of the [CMSIS DSP library](https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html).

## MATLAB code
### Smearing algorithm
Two main scripts are used to test the smearing algorithm with `frequency_smearing.m` and without `test_frequency_smearing.m` block-processing. They both use the following functions.

* `Y = smearing(X, A_s)` Smears the power septrum `X` with the smearing matrix `A_s` previously calculated with`A_s = calc_smear_matric(fs, N, b)` 
* `A_s = calc_smear_matric(fs, N, b)` Calculates the smearing matrix `A_s` of size (N/2 * N/2), with N beieng the length of the frame to process. It uses the auditory filter calculated by `A_s = calc_audit_filter(fs, N, b)`. `b>1` is the smearing coefficient, (`b=1` means no smearing)

The script `Moore_comparison.m` is used to compare the algorithm with results presented by Baer and Moore in [1].
### Data generation for C-code
As the smearing matrix is unique for given parameters, it is first calculated in MATLAB and hard coded in the memory of the TEENSY by simply defining arrays. As memory usage is crucial in an embedded system, the row-indexed sparse storage method (described in [2]) was used to store the matrix's coefficient in a more judicious way.
This is done with the scripts `generate_smear_matrix.m` that prints the coefficient in a text file using either row-indexed sparse storage or classic storage. The content of this file can then be copy-pasted in the C-code (in `smear_mat.h`) to declare the arrays containing the smearing coefficient corresponding to the desired parameters. For instance the following parameters:
```
fs = 16e3;			% sampling frequency
N  = 4;				  % frame length
b  = 6;				  % smearing coefficient
tol = 1e-6;			% thershold used for compressed storage
```
will output this line of code for classical storage:
```
float smear_mat_b6[2][2] = {{1.000000, 0.000000}, {0.00000, 1.000000}};
```
and those two lines for row-indexed sparse storage:
```
unsigned int ija_b6[3] = {3, 3, 3};
float sa_b6[2] = {1.000000, 1.000000};
```

# Implementation on TEENSY
The programm essentially executes three main tasks:
* **Real-time block processing**. This take part of the whole data flow, from fetching data from the audio shield, managing input and output circular buffers and sending the data back to the audio shield. In total, four buffers are used as illustrated by the diagramm below: 
![picture](pictures/dataFlow.png)
* **FFT and IFFT transforms**. The CMSIS DSP library is used for all transforms functions as well as for vector operations. Note that for compatibility with the audio library an older version (1.1.0) must be used. Therefore it is not needed to install the full library, one can simply include `arm_math.h` that comes with the installation of TEENSY.
* **Smearing algorithm**. The smearing matrices are calculated in MATLAB and hard-coded in the memory. Therefore the smearing functions is simply a matrix multiplication. However to save space in the limited memory, the row-index sparse storage method is used.





## References
1. Thomas Baer and Brian C. J. Moore. _“Effects of Spectral Smearing on the Intelligibility of Sentences in Noise”_. en. In:The Journal of the Acoustical Society of America 94.3 (Sept. 1993), pp. 1229–1241.
2. William H. Press et al. _Numerical Recipes in C: The Art of Scientific Computing, Second Edition._ English. 2 edition. Cambridge ; New York: Cambridge University Press, Oct. 1992

