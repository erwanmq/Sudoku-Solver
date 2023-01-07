#include <algorithm>
#include <baseapi.h>
#include <allheaders.h>

#include <opencv2/core/utils/logger.hpp> // for delete the messages in the console

#include "Sudoku.h"


int main(int argc, const char* argv[])
{
	// doesn't print the different messages in the console
	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

	/*if (argc != 2)
	{
		std::cout << "Enter one sudoku image\n";
		std::cin.get();
		return -1;
	}*/


	cv::Mat sudoku_image;
	sudoku_image = cv::imread("sudoku.png"); // read the image

	if (sudoku_image.empty()) // check if the image was load correctly
	{
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return -1;
	}

	Sudo::Sudoku* sudoku = new Sudo::Sudoku;
	if (!sudoku->setImage(sudoku_image))
	{
		std::cout << "Couldn't load the image\n";
		return -1;
	}
	sudoku->initGrid();
	sudoku->getSquares();
	if (!sudoku->detectNumbers())
	{
		std::cout << "Couldn't find numbers\n";
		return -1;
	}
	sudoku->resolveSudoku();

	sudoku->printResult();
	sudoku->printResultImage();
	return 0;
}