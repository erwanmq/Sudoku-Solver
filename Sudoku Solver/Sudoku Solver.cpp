#include <iostream>
#include <vector>
#include <map>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp> // for delete the messages in the console


bool checkRow(int number, std::vector<int>& _vector, int index)
{
	for (int i = 0; i < 9; i++)
	{
		if (_vector[i * 9] == number && i * 9 != index)
			return true;
	}
	return false;
}

bool checkCol(int number, std::vector<int>& _vector, int index)
{
	for (int i = 0; i < 9; i++)
	{
		if (_vector[i] == number && i != index)
			return true;
	}
	return false;
}

void resolveSudoku(std::vector<int>& _vector_num_sudoku)
{
	std::vector<int> numbers_change;
	numbers_change = _vector_num_sudoku;
	int listNumbers[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	for (size_t i = 0; i < numbers_change.size(); i++)
	{
		if (numbers_change[i] == -1)
		{
			int number = 1;
		}
	}
}

void saveSquares(const cv::Mat& _sudoku_image, std::vector<cv::Mat>& _vector_to_save)
{
	cv::Size sudoku_size = _sudoku_image.size(); // get the size of the image

	int size_width_square = sudoku_size.width / 9; // calcul the width of one square
	int size_height_square = sudoku_size.height / 9; // calcul the height of one square

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cv::Mat one_square; // to save the square
			int precision = 5;
			cv::Rect r(i * size_width_square + precision, j * size_height_square + precision, size_width_square - precision, size_height_square - precision); // size of a square
			one_square = _sudoku_image(r).clone(); // copy
			if (one_square.empty())
			{
				std::cout << "error while checking the squares\n";
				return;
			}
			_vector_to_save.push_back(one_square);
		}
		
	}
}

bool isNumber(cv::Mat& _image)
{
	cv::Mat image_clone = _image.clone();
	
	cv::cvtColor(image_clone, image_clone, cv::COLOR_BGR2GRAY);
	cv::threshold(image_clone, image_clone, 140, 255, cv::THRESH_BINARY_INV);

	int val = cv::countNonZero(image_clone);
	if (val >= 600)
		return true;
	return false;
}

bool checkNine(cv::Mat& _image)
{
	int precision = 0;
	cv::Mat half_upper_image = _image(cv::Rect(0, 0, _image.size().width, _image.size().height / 2 + precision)).clone();
	cv::Mat half_lower_image = _image(cv::Rect(0, _image.size().height / 2 + precision, _image.size().width, _image.size().height / 2 - precision)).clone();

	cv::imshow("half", half_lower_image);
	cv::waitKey(200);

	cv::cvtColor(half_upper_image, half_upper_image, cv::COLOR_BGR2GRAY);
	cv::cvtColor(half_lower_image, half_lower_image, cv::COLOR_BGR2GRAY);

	cv::threshold(half_upper_image, half_upper_image, 140, 255, cv::THRESH_BINARY_INV);
	cv::threshold(half_lower_image, half_lower_image, 140, 255, cv::THRESH_BINARY_INV);

	int val1 = cv::countNonZero(half_upper_image);
	int val2= cv::countNonZero(half_lower_image);

	if (val1 > val2)
		return true;

	return false;

}


using contour = std::vector<std::vector<cv::Point>>;
void findMainContours(std::vector<contour>& _storage_contours)
{
	contour main_contours; // contours of main numbers (1-9)
	for (int i = 1; i < 10; i++)
	{
		cv::Mat image;
		cv::String name = "numbers/" + std::to_string(i) + ".png"; // open the number images to get the contours
		image = cv::imread(name);

		image = image(cv::Rect(0, image.size().height / 3 - 20, image.size().width, image.size().height / 3 + 20)); // search the contours of 1/3 + 40 of the images (middle of the images)

		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY); // convert them to gray color
		cv::threshold(image, image, 128, 255, cv::THRESH_BINARY); // convert the images to binary images


		cv::findContours(image, main_contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE); // find the contours
		_storage_contours.push_back(main_contours); // add them to the vector of contours
	}
}

void findSudokuContours(std::vector<contour>& _storage_contours, std::vector<cv::Mat>& _vector_of_images)
{
	contour contours_of_sudoku_numbers; // contour of the numbers in the sudoku
	for (size_t i = 0; i < _vector_of_images.size(); i++) // loop in all the squares of the sudoku (top to bottom and left to right)
	{
		cv::Mat image;
		image = _vector_of_images[i]; // store the image of one square
		image = image(cv::Rect(0, image.size().height / 3 - 20, image.size().width, image.size().height / 3 + 20)); // keep the same size as the first contour detection
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY); // convert image to gray color
		cv::threshold(image, image, 128, 255, cv::THRESH_BINARY); // and convert it to binary image

		cv::findContours(image, contours_of_sudoku_numbers, cv::RETR_LIST, cv::CHAIN_APPROX_NONE); // find the contours
		_storage_contours.push_back(contours_of_sudoku_numbers); // ad them to the vector
	}
}

void compareContours(std::vector<contour>& _storage_contours, std::vector<contour> _storage_main_contours, std::vector<cv::Mat>& _vector_of_images, std::vector<int>& _vec_number_in_sudoku)
{
	std::map<int, double> value_shapes; // map to keep each numbers to his probability
	for (size_t i = 0; i < _storage_contours.size(); i++) // loop in every square of the sudoku
	{
		if (isNumber(_vector_of_images[i])) // check if the square contains a number
		{
			for (size_t j = 0; j < _storage_main_contours.size(); j++) // loop 1 to 9
			{
				double result; // keep the result
				result = cv::matchShapes(_storage_contours[i][0], _storage_main_contours[j][0], 1, 0.0); // method to compare two contours

				value_shapes[j + 1] = result; // keep the result to his key 

			}
			// method to return the smallest number (result). It's mean that it is the right number
			std::map<int, double>::iterator it = std::min_element(value_shapes.begin(), value_shapes.end(),
				[](const auto& l, const auto& r) { return l.second < r.second; });


			//std::cout << it->first << '\n'; // print the number find
			_vec_number_in_sudoku.push_back(it->first); // add it to the vector

		}
		else
			_vec_number_in_sudoku.push_back(-1); //.if the square doesn't contains a number, it stores -1 (empty square)
	}
}

std::vector<int> findNumbers(std::vector<cv::Mat>& _vector)
{
	std::vector<contour> vec_of_main_contours; // vector of the contours before
	findMainContours(vec_of_main_contours);

	std::vector<contour> vec_contours; // and the vector to store them
	findSudokuContours(vec_contours, _vector);

	// this methods will compare every contours of the sudoku to all numbers (1-9)
	// we will use a opencv method that return a double. Smaller is the double, higher is the probability of the right number
	std::vector<int> numbers_in_sudoku; // will store the number that is the closest about the contours
	compareContours(vec_contours, vec_of_main_contours, _vector, numbers_in_sudoku);
	
	
	return numbers_in_sudoku; // return the vector
	
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
	sudoku_image = cv::imread("sudoku.png"); // read the image

	if (sudoku_image.empty()) // check if the image was load correctly
	{
		std::cout << "Could not open or find the image" << std::endl;
		std::cin.get(); //wait for any key press
		return -1;
	}
	
	std::vector<cv::Mat> vec_squares; // vector to save each square of the sudoku	
	saveSquares(sudoku_image, vec_squares);
	
	std::vector<int> numbers_in_sudoku = findNumbers(vec_squares);

	resolveSudoku(numbers_in_sudoku);

	// Creation window
	cv::String windowName = "image"; // name the window
	cv::namedWindow(windowName, cv::WINDOW_NORMAL); // create the window
	
	cv::imshow(windowName, sudoku_image); // show the image in the window
	
	cv::waitKey(0); // wait for user entry
	
	cv::destroyWindow(windowName); //destroy the created window
	
	return 0;
}