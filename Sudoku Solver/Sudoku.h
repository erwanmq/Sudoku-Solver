#pragma once
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>


namespace Sudo
{


typedef std::vector<std::vector<cv::Point>> contours_t;

class Sudoku
{
private:
	cv::Mat m_image_sudoku; // main_image of the sudoku
	cv::Size m_size_sudoku; // size of the sudoku

	cv::Mat m_image_gray_thresh = cv::Mat(); // image that store the sudoku gray and threshold
	cv::Mat m_image_edges = cv::Mat(); // image that stores the edges of the sudoku

	contours_t m_contours{}; // variable that stores the contours of the grid
	std::vector<std::vector<cv::Mat>> m_squares_vector{}; // vector that stores each squares of the grid
	std::vector<std::vector<int>> m_numbers_sudoku{}; // vector that stores each numbers of the grid
	std::vector<std::vector<int>> m_numbers_solutions{}; // vector that stores the solution



public:
	Sudoku();

	const void initGrid();
	const bool setImage(cv::Mat& _image);
	const void getSquares();
	const bool detectNumbers();
	const void resolveSudoku();
	const void printResult();
	const void printResultImage();

private:
	const void getContours();
	const void convertBGRtoGrayThreshold();
	const void getEdges();
	const void saveSquaresToVector();
	const void isANumber(std::string _text, int _col);
	const bool unique_number(int _number, int _row, int _col);
	const bool checkBox(int _number, int _row, int _col);
	const bool checkCol(int _number, int _row, int _col);
	const bool checkRow(int _number, int _row, int _col);
};



}


