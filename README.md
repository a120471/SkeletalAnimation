mesh deformation project (skeletal animation)

Given a mesh object, let's say a human-like mesh for example. Our goal is to manipulate its deformation according to human behavior.

Our inputs include both a well created mesh and a skeleton made for it. Then we use kinect to detect human pose in real time, and calculate the correspondence between different skeletons.

The difficulty may exist in following aspects, one is that we may have errors corresponding between skeletons. In that case, the mesh can bot be transformed to the correct position.

<a href="final result"><img src="https://cloud.githubusercontent.com/assets/4888418/22627005/7ced4668-ebf4-11e6-938d-79a5d63d5474.png" align="center" height="300" width="450" ></a>
