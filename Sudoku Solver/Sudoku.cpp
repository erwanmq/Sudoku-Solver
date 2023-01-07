#include "Sudoku.h"

#include <algorithm>
#include <baseapi.h>
#include <allheaders.h>


using namespace Sudo;

Sudoku::Sudoku() // constructor of the class Sudoku
{}

/// <summary>
/// Initialize the grid. Get the contour of the main grid and crop the image
/// </summary>
/// <returns></returns>
const void Sudoku::initGrid()
{
	getContours();
	if (m_contours.size() == 1) // if contour = 1 means that we can crop the grid (the main grid is smaller than the image)
	{
		cv::Rect rect = cv::boundingRect(m_contours[0]); // create a rect
		m_image_sudoku = m_image_sudoku(rect); // crop the image
	}
}

const bool Sudoku::setImage(cv::Mat& _image)
{
	m_image_sudoku = _image.clone();
	if (m_image_sudoku.empty())
	{
		return false;
	}
	return true;
}

const void Sudoku::saveSquaresToVector()
{
	m_squares_vector.resize(m_contours.size() / 9); // resize the vector to the size of a column (9)
	assert(m_squares_vector.size() == 9);
	int idx = 0;
	for (size_t col = 0; col < m_squares_vector.size(); col++) // loop through the columns
	{
		for (size_t row = 0; row < m_squares_vector.size(); row++) // loop through the rows
		{
			cv::Rect rect = cv::boundingRect(m_contours[idx]); // create a rect to crop the image
			/*cv::imshow("thresh", m_image_gray_thresh(rect));
			cv::waitKey(0);*/
			m_squares_vector[col].push_back(m_image_gray_thresh(rect)); // push back to the vector the image croped
			idx++;
		}
	}
}

const void Sudoku::getSquares()
{
	getContours(); // save the squares of the grid in the vector contours

	// sort the contours from the upper left corner to the lower right corner. It's not precise for the y coordinates.
	std::sort(m_contours.begin(), m_contours.end(), [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
		return c1[0].x < c2[0].x || (c1[0].x == c2[0].x && c1[0].y < c2[0].y); });

	int idx = 0;
	for (const auto& contour : m_contours)
	{
		cv::drawContours(m_image_sudoku, m_contours, idx, cv::Scalar(0, 0, 255), 2);
		idx++;
		/*cv::imshow("contours", m_image_sudoku);
		cv::waitKey(50);*/
	}

	// so we loop through a certain amount of the vector. We loop in each column of the vector and sort the contour from the upper grid to the lower grid.
	// 3x3 grid
	int numbers_row_squares = 9; // number of squares in a row
	int numbers_col_squares = 9; // number of squares in a column
	for (int start = 0; start < numbers_row_squares; start++)
	{
		// sort from the upper to the lower grid
		std::sort(m_contours.begin() + start * numbers_row_squares + 1, m_contours.begin() + start * numbers_row_squares + numbers_col_squares, 
			[](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
			return c1[0].y < c2[0].y;
		});
	}
	
	idx = 0;
	for (const auto& contour : m_contours)
	{
		cv::drawContours(m_image_sudoku, m_contours, idx, cv::Scalar(0, 255, 0), 2);
		idx++;
		/*cv::imshow("contours", m_image_sudoku);
		cv::waitKey(50);*/
	}	

	saveSquaresToVector();
}

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

const bool Sudo::Sudoku::detectNumbers()
{
	tesseract::TessBaseAPI* tess = new tesseract::TessBaseAPI;
	if (tess->Init("D:/C++ Library/Tesseract/tessdata", "eng", tesseract::OEM_LSTM_ONLY))
	{
		std::cout << "Fjailed loading tesseract\n";
		return false;
	}
	tess->SetPageSegMode(tesseract::PSM_RAW_LINE);

	m_numbers_sudoku.resize(m_squares_vector.size());
	for (size_t col = 0; col < m_squares_vector.size(); col++)
	{
		for (size_t row = 0; row < m_squares_vector[col].size(); row++)
		{
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::dilate(m_squares_vector[col][row], m_squares_vector[col][row], kernel);
			cv::erode(m_squares_vector[col][row], m_squares_vector[col][row], kernel);

			cv::cvtColor(m_squares_vector[col][row], m_squares_vector[col][row], cv::COLOR_BGR2RGBA); // convert the image to RGBA to be used by tesseract
			tess->SetImage(m_squares_vector[col][row].data, m_squares_vector[col][row].cols, m_squares_vector[col][row].rows, 4, m_squares_vector[col][row].step); // set the Image
			//cv::imshow("tess", m_squares_vector[col][row]);

			std::string outText = std::string(tess->GetUTF8Text()); // get the recognized text
			//std::cout << outText << '\n';

			isANumber(outText, col);
		}
		
	}

	tess->End();

	if (m_numbers_sudoku.size() == 9 && m_numbers_sudoku[0].size() == 9)
		return true;
	
	return false;

}

/// <summary>
/// Function that find the contours and return the vector
/// </summary>
/// <returns></returns>
const void Sudoku::getContours()
{
	getEdges(); // get the edges to the member variable
	cv::findContours(m_image_edges, m_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // find the contours
}

/// <summary>
/// Convert the image to gray and threshold it
/// </summary>
/// <returns></returns>
const void Sudoku::convertBGRtoGrayThreshold()
{
	cv::cvtColor(m_image_sudoku, m_image_gray_thresh, cv::COLOR_BGR2GRAY); // convert color to gray
	cv::threshold(m_image_gray_thresh, m_image_gray_thresh, 180, 255, cv::THRESH_BINARY); // threshold the image
}

/// <summary>
/// Find the edges of the image
/// </summary>
/// <returns></returns>
const void Sudoku::getEdges()
{
	convertBGRtoGrayThreshold(); // convert to gray and threshold the member variable
	cv::Mat image; // stores the modifications

	cv::GaussianBlur(m_image_gray_thresh, image, cv::Size(5, 5), 0); 
	cv::erode(image, image, cv::Mat());
	cv::dilate(image, image, cv::Mat());

	cv::Canny(image, m_image_edges, 50, 255, 3); // find the edges and output it in the member variable
}



const bool Sudoku::checkRow(int _number, int _row, int _col)
{
	for (size_t i = 0; i < m_numbers_solutions.size(); i++)
	{
		if (m_numbers_solutions[i][_row] == _number && i != _col)
			return true;
	}
	return false;
}

const bool Sudoku::checkCol(int _number, int _row, int _col)
{
	for (size_t i = 0; i < m_numbers_solutions[_col].size(); i++)
	{
		if (m_numbers_solutions[_col][i] == _number && i != _row)
			return true;
	}
	return false;
}


const bool Sudoku::checkBox(int _number, int _row, int _col)
{
	int row_modulo = _row % 3;
	int col_modulo = _col % 3;

	for (int i = _col - col_modulo; i < _col - col_modulo + 3; i++)
	{
		for (int j = _row - row_modulo; j < _row - row_modulo + 3; j++)
		{
			if (m_numbers_solutions[i][j] == _number && i != _col && j != _row) // vrai && faux && vrai
				return true;
		}
	}
	return false;

}

const bool Sudoku::unique_number(int _number, int _row, int _col)
{
	if (checkBox(_number, _row, _col) || checkRow(_number, _row, _col) || checkCol(_number, _row, _col))
		return false;

	return true;
}


const void Sudoku::resolveSudoku()
{
	m_numbers_solutions = m_numbers_sudoku;
	
	for (int col = 0; col < m_numbers_solutions.size(); col++)
	{
		for (int row = 0; row < m_numbers_solutions[col].size(); row++)
		{
			if (m_numbers_sudoku[col][row] == 0)
			{
				int number = 1;

				while (!unique_number(number, row, col) || number > 9)
				{
					if (number < 9)
						number++;

					else {
						do {
							if (m_numbers_sudoku[col][row] == 0)
								m_numbers_solutions[col][row] = 0;
							if (row > 0)
								row--;
							else {
								row = 8;
								if(col > 0)
									col--;
							}
							number = m_numbers_solutions[col][row] + 1;
						} while (m_numbers_sudoku[col][row] != 0);

					}
				}
				m_numbers_solutions[col][row] = number;

			}

		}

	}
}

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

const void Sudo::Sudoku::printResultImage()
{
	
	int idx = 0;
	for (size_t i = 0; i < m_squares_vector.size(); i++)
	{
		for (size_t j = 0; j < m_squares_vector[i].size(); j++)
		{
			cv::Rect rect = cv::boundingRect(m_contours[idx]);
			int width_square = rect.size().width;
			int height_square = rect.size().height;
			double size_font = 5.0;
			cv::Point position = cv::Point(m_contours[idx][0].x + (width_square - size_font * 1.5) / 2.f, m_contours[idx][0].y + (height_square) / 2.f);
			if (m_numbers_sudoku[i][j] == 0)
			{
				cv::putText(m_image_sudoku, std::to_string(m_numbers_solutions[i][j]), position, cv::FONT_HERSHEY_PLAIN, size_font, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
			}
			idx++;
		}
	}
	cv::namedWindow("solution", cv::WINDOW_NORMAL);
	cv::imshow("solution", m_image_sudoku);
	cv::waitKey(0);
}
