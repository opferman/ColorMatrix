# Color Matrix

## About

This application searches an array of colors, a matrix, and finds the largest region of the same color and returns the count.

### Version 1

This version has a main thread which itterates through each cell and spawns a thread everytime it sees a new color.  The thread then itterates through that region and counts.

Finally, it parses the data and returns the result by itterating through each region.

### Version 2

This version the main thread is part of the search.  Everytime it hits a new color it will spawn another thread that does the same.  In this manner only a few more pixels than checking every pixel is actually performed.  This is complete once all pixels have been checked.


