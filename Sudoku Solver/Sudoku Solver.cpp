#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>


#include <baseapi.h>
#include <allheaders.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp> // for delete the messages in the console


bool checkRow(int _number, std::vector<std::vector<int>>& _vector, int _row, int _col)
{
	for (size_t i = 0; i < _vector.size(); i++)
	{
		if (_vector[i][_row] == _number && i != _col)
			return true;
	}
	return false;
}

bool checkCol(int _number, std::vector<std::vector<int>>& _vector, int _row, int _col)
{
	for (size_t i = 0; i < _vector[_col].size(); i++)
	{
		if (_vector[_col][i] == _number && i != _row)
			return true;
	}
	return false;
}


bool checkBox(int _number, std::vector<std::vector<int>>& _vector, int _row, int _col)
{
	int row_modulo = _row % 3;
	int col_modulo = _col % 3;

	for (int i = _col - col_modulo; i < _col - col_modulo + 3; i++) 
	{
		for (int j = _row - row_modulo; j < _row - row_modulo + 3; j++)
		{
			if (_vector[i][j] == _number && i != _col && j != _row) // vrai && faux && vrai
				return true;
		}
	}
	return false;

}

bool unique_number(int _number, std::vector<std::vector<int>>& _vector, int _row, int _col)
{
	if (checkBox(_number, _vector, _row, _col) || checkRow(_number, _vector, _row, _col) || checkCol(_number, _vector, _row, _col))
		return false;

	return true;
}

std::vector<std::vector<int>> resolveSudoku(std::vector<std::vector<int>>& _vector_num_sudoku)
{

	std::vector<std::vector<int>> numbers_change(_vector_num_sudoku);
	for (int col = 0; col < numbers_change.size(); col++)
	{
		for (int row = 0; row < numbers_change[col].size(); row++)
		{
			
			if (_vector_num_sudoku[col][row] == 0)
			{
				int number = 1;
				
				while (!unique_number(number, numbers_change, row, col) || number > 9)
				{
					if (number < 9)
						number++;

					else{
						do {
							if (_vector_num_sudoku[col][row] == 0)
								numbers_change[col][row] = 0;
							if (row > 0)
								row--;
							else {
								row = 8;
								col--;
							}
							number = numbers_change[col][row] + 1;
						} while (_vector_num_sudoku[col][row] != 0);
						
					}
				}
				numbers_change[col][row] = number;

			}
			
		}
		
	}

	return numbers_change;
}

std::vector<std::vector<int>> findNumbers(std::vector<std::vector<cv::Mat>>& _vector_image)
{
	std::vector<std::vector<int>> numbers; // vector which stores the numbers of the sudoku
	numbers.resize(9);

	tesseract::TessBaseAPI* tess = new tesseract::TessBaseAPI; // tesseract to find numbers
	if (tess->Init("D:/C++ Library/Tesseract/tessdata", "eng", tesseract::OEM_LSTM_ONLY)) // initialization
	{
		std::cout << "couldn't initialize tesseract.\n"; // error
		return {};
	}
	tess->SetPageSegMode(tesseract::PSM_RAW_LINE); // search for a few character

	for (size_t i = 0; i < _vector_image.size(); i++) // loop in all images
	{
		for (size_t j = 0; j < _vector_image[i].size(); j++)
		{
			cv::resize(_vector_image[i][j], _vector_image[i][j], cv::Size(200, 200));
			cv::threshold(_vector_image[i][j], _vector_image[i][j], 100, 255, cv::THRESH_BINARY);

			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			//cv::dilate(_vector_image[i][j], _vector_image[i][j], kernel);
			cv::erode(_vector_image[i][j], _vector_image[i][j], kernel);


			cv::cvtColor(_vector_image[i][j], _vector_image[i][j], cv::COLOR_BGR2RGBA); // convert the image to RGBA to be used by tesseract

			tess->SetImage(_vector_image[i][j].data, _vector_image[i][j].cols, _vector_image[i][j].rows, 4, _vector_image[i][j].step); // set the Image

			std::string outText = std::string(tess->GetUTF8Text()); // get the recognized text

			//std::cout << outText << '\n';
			// method to extract a digit from a string
			std::stringstream s;
			s << outText; // put the string to the stringstream

			std::cout << outText << '\n';
			cv::waitKey(500);

			std::string temp; // temp variable
			bool find = false; // this boolean will tell to our code if there is a number or not
			int found;
			while (!s.eof() && !find) // loop until the end of the string
			{
				s >> temp; // give a character to our temp variable

				if (std::stringstream(temp) >> found) // check if the character is a digit
				{
					numbers[i].push_back(found); // if it is, push back it in the vector
					find = true; // we find a number
				}
				cv::imshow("image", _vector_image[i][j]);
				
				temp = ""; // reset the variable
			}
			if (!find) // if there is no number
				numbers[i].push_back(0); // push back 0
		}		

	}
	tess->End(); // destroy the tesseract
	 
	return numbers; // return the vector
}

std::vector<std::vector<int>> saveSquares(const cv::Mat& _sudoku_image)
{
	std::vector<std::vector<cv::Mat>> vector_to_save;
	vector_to_save.resize(9);
	cv::Size sudoku_size = _sudoku_image.size(); // get the size of the image

	int size_width_square = sudoku_size.width / 9; // calcul the width of one square
	int size_height_square = sudoku_size.height / 9; // calcul the height of one square

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cv::Mat one_square; // to save the square
			int precision = 15;
			cv::Rect r(i * size_width_square + precision, j * size_height_square + precision, size_width_square - precision, size_height_square - precision); // size of a square
			one_square = _sudoku_image(r).clone(); // copy
			if (one_square.empty())
			{
				std::cout << "error while checking the squares\n";
				return{};
			}
			vector_to_save[i].push_back(one_square);
		}
		
	}

	std::vector<std::vector<int>> numbers = findNumbers(vector_to_save);

	return numbers;
}

std::vector<cv::Vec2f> detectContourLine(cv::Mat& _image)
{
	cv::Mat image_modify = _image.clone();
	cv::GaussianBlur(image_modify, image_modify, cv::Size(5, 5), 0);
	cv::erode(image_modify, image_modify, cv::Mat());
	cv::dilate(image_modify, image_modify, cv::Mat());


	cv::Mat edge_detection;
	cv::Canny(image_modify, edge_detection, 50, 255, 3);

	std::vector<cv::Vec2f> lines;
	cv::HoughLines(edge_detection, lines, 1, CV_PI / 180, 200, 0, 0);

	return lines;
}

void getTheGrid(std::vector<cv::Vec2f>& _lines, cv::Mat& _image)
{
	int number = 0;
	std::vector<int> vertical_lines;
	std::vector<int> horizontal_lines;

	for (size_t i = 0; i < _lines.size(); i++)
	{
		float theta = _lines[i][1];
		if (theta < CV_PI / 4 || theta > 3 * CV_PI / 4)
		{
			float rho = _lines[i][0];
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;

			pt1.x = cvRound(x0 + _image.size().width * (-b));
			pt1.y = cvRound(y0 + _image.size().height * a);
			pt2.x = cvRound(x0 - _image.size().width * (-b));
			pt2.y = cvRound(y0 - _image.size().height * a);

			vertical_lines.push_back(pt1.x);			
		}

		if (theta > CV_PI / 3 && theta < 3 * CV_PI / 3)
		{
			float rho = _lines[i][0];
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;

			pt1.x = cvRound(x0 + _image.size().width * (-b));
			pt1.y = cvRound(y0 + _image.size().height * a) < 0 ? 0 : cvRound(y0 + _image.size().height * a);
			pt2.x = cvRound(x0 - _image.size().width * (-b));
			pt2.y = cvRound(y0 - _image.size().height * a);

			horizontal_lines.push_back(pt1.y);			
		}
		
	}

	std::vector<int>::iterator extLeftLineIndex = std::min_element(vertical_lines.begin(), vertical_lines.end());
	vertical_lines.erase(extLeftLineIndex);
	int innerLeftLineIndex = *std::min_element(vertical_lines.begin(), vertical_lines.end());

	std::vector<int>::iterator extRightLineIndex = std::max_element(vertical_lines.begin(), vertical_lines.end());
	vertical_lines.erase(extRightLineIndex);
	int innerRightLineIndex = *std::max_element(vertical_lines.begin(), vertical_lines.end());


	std::vector<int>::iterator extTopPosition = std::min_element(horizontal_lines.begin(), horizontal_lines.end());
	horizontal_lines.erase(extTopPosition);
	int innerTopPosition = *std::min_element(horizontal_lines.begin(), horizontal_lines.end());

	std::vector<int>::iterator extBotPosition = std::max_element(horizontal_lines.begin(), horizontal_lines.end());
	horizontal_lines.erase(extBotPosition);
	int innerBotPosition = *std::max_element(horizontal_lines.begin(), horizontal_lines.end());
	

	cv::Rect crop_image(innerLeftLineIndex, innerTopPosition, innerRightLineIndex - innerLeftLineIndex, innerBotPosition - innerTopPosition);

	_image = _image(crop_image);

	cv::String windowName = "image"; // name the window
	cv::namedWindow(windowName, cv::WINDOW_NORMAL); // create the window

	cv::imshow(windowName, _image);

	cv::waitKey(0);

}

void writeAnswer(std::vector<std::vector<int>>& _numbers_before, std::vector<std::vector<int>>& _resolved, cv::Mat& _image)
{
	cv::Size sudoku_size = _image.size(); // get the size of the image

	int size_width_square = sudoku_size.width / 9; // calcul the width of one square
	int size_height_square = sudoku_size.height / 9; // calcul the height of one square

	for (size_t i = 0; i < _numbers_before.size(); i++)
	{
		for (size_t j = 0; j < _numbers_before[i].size(); j++)
		{
			cv::Point position = cv::Point(size_width_square * i + size_width_square *0.8/ 2.f, size_height_square * j + size_height_square * 1.3 / 2.f);
			if (_numbers_before[i][j] == 0)
			{
				cv::putText(_image, std::to_string(_resolved[i][j]), position, cv::FONT_HERSHEY_PLAIN, 5.0, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
			}
		}
	}
}

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
	sudoku_image = cv::imread("sudoku_test_2.png"); // read the image

	if (sudoku_image.empty()) // check if the image was load correctly
	{
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return -1;
	}

	if (sudoku_image.cols <= 1200)
	{
		cv::resize(sudoku_image, sudoku_image, cv::Size(1200, 1200), cv::INTER_LINEAR);
	}

	cv::Mat sudoku_image_gray;
	cv::cvtColor(sudoku_image, sudoku_image_gray, cv::COLOR_BGR2GRAY);

	cv::Mat sudoku_image_threshold;
	cv::threshold(sudoku_image_gray, sudoku_image_threshold, 128, 255, cv::THRESH_BINARY);

	std::vector<cv::Vec2f> lines = detectContourLine(sudoku_image_threshold);

	getTheGrid(lines, sudoku_image);

	std::vector<std::vector<int>> numbers = saveSquares(sudoku_image);

	


	if (numbers.empty())
	{
		std::cout << "error when checking the numbers\n";
		return -1;
	}

	std::vector<std::vector<int>> resolved = resolveSudoku(numbers);

	writeAnswer(numbers, resolved, sudoku_image);

	/*for (size_t i = 0; i < squares.size(); i++)
	{
		cv::imshow(std::to_string(i), squares[i]);
	}*/


	for (size_t i = 0; i < numbers.size(); i++)
	{
		for (size_t j = 0; j < numbers[i].size(); j++)
			std::cout << resolved[j][i] << " ";

		std::cout << '\n';
	}

	// Creation window
	cv::String windowName = "image"; // name the window
	cv::namedWindow(windowName, cv::WINDOW_NORMAL); // create the window

	cv::imshow(windowName, sudoku_image);



	cv::waitKey(0); // wait for user entry

	cv::destroyWindow(windowName); //destroy the created window












	//std::vector<cv::Mat> vec_squares; // vector to save each square of the sudoku	
	//saveSquares(sudoku_image, vec_squares);
	
	//std::vector<std::vector<int>> numbers_in_sudoku = findNumbers(vec_squares);

	//numbers_in_sudoku = resolveSudoku(numbers_in_sudoku);

	/*for (size_t i = 0; i < numbers_in_sudoku.size(); i++)
	{
		for (size_t j = 0; j < numbers_in_sudoku[i].size(); j++)
		{
			std::cout << numbers_in_sudoku[j][i] << " ";
		}
		std::cout << '\n';
	}
	std::cout << "there is " << number_in_sudoku << " numbers\n";*/

	
	
	return 0;
}