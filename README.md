# Extract RGB-D images

With the aid of the Azure Kinect SDK, RGB-D images are extracted out of MKV recordings that are generated with the Azure Kinect Recorder. These recordings are played back and color and depth images are extracted. Depth images are transformed into the coordinate system of the color images and both, tranformed depth and color images, are extracted at each time step. Usage is explained at the beginning of the script. 
