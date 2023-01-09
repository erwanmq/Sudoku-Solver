#include "Sudoku.h"

#include <algorithm>
#include <baseapi.h>
#include <allheaders.h>


using namespace Sudo;

Sudoku::Sudoku() // constructor of the class Sudoku
{}

Sudo::Sudoku::Sudoku(cv::Mat& _image)
{
	setImage(_image);
}

Sudoku::~Sudoku()
{}

// Initialize the grid. Get the contour of the main grid and crop the image
const void Sudoku::initGrid()
{
	getContours(m_image_sudoku);
	if (m_contours.size() == 1) // if contour = 1 means that we can crop the grid (the main grid is smaller than the image)
	{
		cv::Rect rect = cv::boundingRect(m_contours[0]); // create a rect
		m_image_sudoku = m_image_sudoku(rect); // crop the image
	}
}

// set the image to the sudoku variable
const bool Sudoku::setImage(cv::Mat& _image)
{
	m_image_sudoku = _image.clone();
	if (m_image_sudoku.empty() && m_image_sudoku.channels() == 3) // check if the image is compatible and not empty
	{
		return false;
	}
	return true;
}

// function that store the image of each square to a vector 
const void Sudoku::saveSquaresToVector()
{
	m_squares_vector.resize(m_contours.size() / 9); // resize the vector to the size of a column (9)
	assert(m_squares_vector.size() == 9); // check if m_contours.size() has 82 contours (82 squares for 3x3 sudoku)

	int idx = 0; // index for the unidimensionnal vector
	for (size_t col = 0; col < m_squares_vector.size(); col++) // loop through the columns
	{
		for (size_t row = 0; row < m_squares_vector.size(); row++) // loop through the rows
		{
			cv::Rect rect = cv::boundingRect(m_contours[idx]); // create a rect to crop the image
			m_squares_vector[col].push_back(m_image_gray_thresh(rect)); // push back to the vector the image croped
			idx++;
		}
	}
}

// function that search for each squares of the sudoku
const void Sudoku::getSquares()
{
	getContours(m_image_sudoku); // save the squares of the grid in the vector contours

	// this part makes it possible to approximate a contour in polygon. If the polygon is a square ( size = 4 ) it is a sudoku's square.
	for (int i = 0; i < m_contours.size(); i++) // loop through all contours
	{
		std::vector<cv::Point> polygon; // create a polygon
		cv::approxPolyDP(m_contours[i], polygon, 0.1 * cv::arcLength(m_contours[i], true), true); // approxim a contour to a polygon and store it

		if (polygon.size() != 4) // check if it is a square
		{
			m_contours.erase(m_contours.begin() + i);
		}
	}

	// sort the contours from the upper left corner to the lower right corner. It's not precise for the y coordinates.
	std::sort(m_contours.begin(), m_contours.end(), [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
		return c1[0].x < c2[0].x || (c1[0].x == c2[0].x && c1[0].y < c2[0].y); });

	// so we loop through a certain amount of the vector. We loop in each column of the vector and sort the contour from the upper grid to the lower grid.
	// 3x3 grid
	int numbers_row_squares = 9; // number of squares in a row
	int numbers_col_squares = 9; // number of squares in a column
	for (int start = 0; start < numbers_row_squares; start++)
	{
		// sort from the upper to the lower grid
		std::sort(m_contours.begin() + start * numbers_row_squares, m_contours.begin() + start * numbers_row_squares + numbers_col_squares, 
			[](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
			return c1[0].y < c2[0].y;
		});
	}

	saveSquaresToVector(); // function that stores each squares found to a vector
}

// function that check if the string found by tesseract is a number
// take in parameter the text of tesseract, and the column to push back to a vector the number found
const void Sudoku::isANumber(std::string _text, int _col)
{
	std::stringstream s;
	s << _text; // put the string to the stringstream

	std::string temp; // temp variable
	bool find = false; // this boolean will tell to our code if there is a number or not
	int found;
	while (!s.eof() && !find) // loop until the end of the string
	{
		s >> temp; // give a character to our temp variable

		if (std::stringstream(temp) >> found) // check if the character is a digit
		{
			m_numbers_sudoku[_col].push_back(found); // if it is, push back it in the vector
			find = true; // we find a number
		}

		temp = ""; // reset the variable
	}
	if (!find) // if there is no number
		m_numbers_sudoku[_col].push_back(0); // push back 0
}

// function that use tesseract to find characters
const bool Sudo::Sudoku::detectNumbers()
{
	tesseract::TessBaseAPI* tess = new tesseract::TessBaseAPI; // create the object tesseract
	if (tess->Init("D:/C++ Library/Tesseract/tessdata", "eng", tesseract::OEM_LSTM_ONLY)) // initialize it
	{
		std::cout << "Failed loading tesseract\n";
		return false; // return false if it failed
	}
	tess->SetPageSegMode(tesseract::PSM_RAW_LINE);

	m_numbers_sudoku.resize(m_squares_vector.size()); // resize the vector to the same size of the numbers of squares
	cv::Mat image;
	for (size_t col = 0; col < m_squares_vector.size(); col++) // loop through columns
	{
		for (size_t row = 0; row < m_squares_vector[col].size(); row++) // loop through lines
		{
			// threshold the image and inverse it. The numbers become white on a black background
			cv::threshold(m_squares_vector[col][row], m_squares_vector[col][row], 150, 255, cv::THRESH_BINARY_INV);
			// create a kernel to dilate and erode
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::dilate(m_squares_vector[col][row], m_squares_vector[col][row], kernel);
			cv::erode(m_squares_vector[col][row], m_squares_vector[col][row], kernel);

			cv::cvtColor(m_squares_vector[col][row], m_squares_vector[col][row], cv::COLOR_BGR2RGBA); // convert the image to RGBA to be used by tesseract
			tess->SetImage(m_squares_vector[col][row].data, m_squares_vector[col][row].cols, m_squares_vector[col][row].rows, 4, m_squares_vector[col][row].step); // set the Image

			std::string outText = std::string(tess->GetUTF8Text()); // get the recognized text

			isANumber(outText, col);
		}
		
	}

	tess->End(); // destroy the object

	if (m_numbers_sudoku.size() == 9 && m_numbers_sudoku[0].size() == 9) // if the size is 9. The sudoku seems good
		return true;
	
	return false;

}

// Function that find the contours and return the vector
const void Sudoku::getContours(cv::Mat& _image)
{
	getEdges(_image); // get the edges to the member variable
	cv::findContours(m_image_edges, m_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // find the contours
}

// Convert the image to gray and threshold it
const void Sudoku::convertBGRtoGrayThreshold(cv::Mat& _image)
{
	cv::cvtColor(_image, m_image_gray_thresh, cv::COLOR_BGR2GRAY); // convert color to gray
	cv::threshold(m_image_gray_thresh, m_image_gray_thresh, 200, 255, cv::THRESH_BINARY); // threshold the image
}

// Find the edges of the image
const void Sudoku::getEdges(cv::Mat& _image)
{
	convertBGRtoGrayThreshold(_image); // convert to gray and threshold the member variable
	cv::Mat image; // stores the modifications

	cv::GaussianBlur(m_image_gray_thresh, image, cv::Size(5, 5), 0); 
	cv::erode(image, image, cv::Mat());
	cv::dilate(image, image, cv::Mat());

	cv::Canny(image, m_image_edges, 50, 255, 3); // find the edges and output it in the member variable
}


/*

				PART FOR RESOLVE THE SUDOKU

*/

// check if the number is in the row
const bool Sudoku::checkRow(int _number, int _row, int _col)
{
	for (size_t i = 0; i < m_numbers_solutions.size(); i++) // loop through the column
	{
		if (m_numbers_solutions[i][_row] == _number && i != _col) // check the same index in each column
			return true; // return true if there is the same number
	}
	return false;
}

// check if the number is in the column
const bool Sudoku::checkCol(int _number, int _row, int _col)
{
	for (size_t i = 0; i < m_numbers_solutions[_col].size(); i++)
	{
		if (m_numbers_solutions[_col][i] == _number && i != _row)
			return true;
	}
	return false;
}

// check if the number is in the same box
const bool Sudoku::checkBox(int _number, int _row, int _col)
{
	int row_modulo = _row % 3;
	int col_modulo = _col % 3;

	for (int i = _col - col_modulo; i < _col - col_modulo + 3; i++) // loop through the first square of the box until the last square
	{
		for (int j = _row - row_modulo; j < _row - row_modulo + 3; j++)
		{
			if (m_numbers_solutions[i][j] == _number && i != _col && j != _row)
				return true;
		}
	}
	return false;

}

// function that return false if a function above return true
const bool Sudoku::unique_number(int _number, int _row, int _col)
{
	if (checkBox(_number, _row, _col) || checkRow(_number, _row, _col) || checkCol(_number, _row, _col))
		return false;

	return true;
}

// the main function for resolving the sudoku using backtracking
const void Sudoku::resolveSudoku()
{
	m_numbers_solutions.clear();
	m_numbers_solutions = m_numbers_sudoku; // copy the numbers of the sudoku to the vector that will store the solutions
	
	for (int col = 0; col < m_numbers_solutions.size(); col++) // loop through the columns of the sudoku
	{
		for (int row = 0; row < m_numbers_solutions[col].size(); row++) // loop through the rows
		{
			if (m_numbers_sudoku[col][row] == 0) // if the square is empty (it is a 0) we can add a number
			{
				int number = 1; // the first number we try is 1

				while (!unique_number(number, row, col) || number > 9) // if it is not a unique number in the row, col or box we loop
				{
					if (number < 9)
						number++; // increase the number and check if it is unique

					else { // if the number is above 9, we go back
						do {
							if (m_numbers_sudoku[col][row] == 0) // if the number was a 0 (empty) we put a 0
								m_numbers_solutions[col][row] = 0;
							if (row > 0) // we go back
								row--;
								
							else {
								row = 8; // if row was 0 we go to 8 (bottom of the column)
								if(col > 0)
									col--; // and go back to the previous column
							}
							number = m_numbers_solutions[col][row] + 1; // we change the number of the previous square to + 1
						} while (m_numbers_sudoku[col][row] != 0); // do that until we find a previous empty square

					}
				}
				m_numbers_solutions[col][row] = number; // if all is valid, we change the 0 to the number find
			}
		}

	}
}

// function that print result on the console
const void Sudo::Sudoku::printResult()
{
	for (size_t col = 0; col < m_numbers_solutions.size(); col++)
	{
		for (size_t row = 0; row < m_numbers_solutions[col].size(); row++)
		{
			std::cout << m_numbers_solutions[row][col] << " ";
		}
		std::cout << '\n';
	}
}

// function that print result on the image
const void Sudo::Sudoku::printResultImage()
{
	
	int idx = 0; // index for the unidimensionnal vector of contours
	for (size_t i = 0; i < m_squares_vector.size(); i++)
	{
		for (size_t j = 0; j < m_squares_vector[i].size(); j++)
		{
			cv::Rect rect = cv::boundingRect(m_contours[idx]); // create a rect of a contour
			// keep its size 
			int width_square = rect.size().width; 
			int height_square = rect.size().height;

			// create a font for write on the image (random value find by testing)
			double size_font = 5.0 * m_image_sudoku.size().width / 1200;
			// position of the text (a little bit in the middle of the contour)
			cv::Point position = cv::Point(m_contours[idx][0].x + (width_square - size_font * 1.5) / 2.f, m_contours[idx][0].y + (height_square) / 2.f);
			if (m_numbers_sudoku[i][j] == 0)
			{
				// write it on the image
				cv::putText(m_image_sudoku, std::to_string(m_numbers_solutions[i][j]), position, cv::FONT_HERSHEY_PLAIN, size_font, cv::Scalar(0, 0, 255), 2, cv::LINE_AA); 
			}
			idx++;
		}
	}
	std::string windowName = "solution";
	cv::namedWindow(windowName, cv::WINDOW_NORMAL);
	cv::imshow(windowName, m_image_sudoku);
	cv::waitKey(0);

	cv::destroyWindow(windowName);
}

const void Sudoku::enterManualNumbers()
{
	std::cout << "Enter the number (top to bot and left to right)>>";
	m_numbers_sudoku.clear();
	m_numbers_sudoku.resize(9);
	for (size_t i = 0; i < m_squares_vector.size(); i++)
	{
		for (size_t j = 0; j < m_squares_vector[i].size(); j++)
		{
			cv::Mat image;
			cv::cvtColor(m_squares_vector[i][j], image, cv::COLOR_RGBA2GRAY);
			//cv::threshold(image, image, 150, 255, cv::THRESH_BINARY_INV);
			int blackPixel = cv::countNonZero(image);
			cv::imshow("square", image);
			cv::waitKey(20);
			if (blackPixel > 500 * m_image_sudoku.size().width / 1200)
			{
				int value;
				std::cin >> value;
				m_numbers_sudoku[i].push_back(value);
				std::cout << ">>";
			}
			else
				m_numbers_sudoku[i].push_back(0);
		}
	}
}
