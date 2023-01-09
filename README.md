# Sudoku Solver
## What is the project?
The Sudoku Solver is a c++ program that allows the user to enter a sudoku image and the program gives in output the solution. The program use OpenCV for the image
processing and Tesseract for numbers recognition.
### Input:
![sudoku2](https://user-images.githubusercontent.com/93351393/211330981-d95aa652-090a-456e-b4c5-bc9a2e0c4939.png)
### Output:
![sudoku_image](https://user-images.githubusercontent.com/93351393/211331298-118b821e-22b7-4d52-b20b-105d9960af4d.png)

### You can see a demo of this program in this youtube video: 
https://www.youtube.com/watch?v=oTrtURw761k

## How do I use it?
To use it, you just have to download the zip file, build the program in your favorite IDE and run it. If you run from the console you just have to put as first 
argument the sudoku image path. If you run it in the ide, you have to comment or delete the 
```
if (argc != 2)
	{
		std::cout << "Enter one sudoku image\n";
		std::cin.get();
		return -1;
	}
 ```
 and remplace the path_image by the sudoku image path.
 
 ## Does it work every time?
 For the moment the sudoku solver has few problems for this part. If the image is clean, big, with no contrast of color it will work.
 But the numbers recognition can failed. That's why I add a manual function that allows the user to enter the differents numbers.
