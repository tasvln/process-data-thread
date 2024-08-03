/*
  Name: Temitope Oladimeji Samuel Adebayo

  I have done all the coding by myself and only copied the code that my professor provided 
  to complete my workshops and assignments.
*/

// Workshop 9 - Multi-Threading, Thread Class
// process_data.cpp
// 2021/1/5 - Jeevan Pant
// 2023/11/17 - Cornel

#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>
#include <thread>
#include "process_data.h"

namespace sdds
{
	// The following function receives array (pointer) as the first argument, number of array 
	//   elements (size) as second argument, divisor as the third argument, and avg as fourth argument. 
	//   size and divisor are not necessarily same. When size and divisor hold same value, avg will 
	//   hold average of the array elements. When they are different, avg will hold a value called 
	// 	 as average-factor. For part 1, you will be using same value for size and double. Use of 
	//   different values for size and divisor will be useful for multi-threaded implementation in part-2. 
	void computeAvgFactor(const int* arr, int size, int divisor, double& avg) {
		avg = 0;
		for (int i = 0; i < size; i++) {
			avg += arr[i];
		}
		avg /= divisor;
	}

	// The following function receives array (pointer) as the first argument, number of array elements  
	//   (size) as second argument, divisor as the third argument, computed average value of the data items
	//   as fourth argument, and var as fifth argument. Size and divisor are not necessarily same as in the 
	//   case of computeAvgFactor. When size and divisor hold same value, var will get total variance of 
	//   the array elements. When they are different, var will hold a value called as variance factor. 
	//   For part 1, you will be using same value for size and double. Use of different values for size 
	//   and divisor will be useful for multi-threaded implementation in part-2. 
	void computeVarFactor(const int* arr, int size, int divisor, double avg, double& var) {
		var = 0;
		for (int i = 0; i < size; i++) {
			var += (arr[i] - avg) * (arr[i] - avg);
		}
		var /= divisor;
	}

	// The following constructor of the functor receives name of the data file, opens it in 
	//   binary mode for reading, reads first int data as total_items, allocate memory space 
	//   to hold the data items, and reads the data items into the allocated memory space. 
	//   It prints first five data items and the last three data items as data samples. 
	//   
	ProcessData::ProcessData(const std::string& filename, int n_threads) {  
		// TODO: Open the file whose name was received as parameter and read the content
		//         into variables "total_items" and "data". Don't forget to allocate
		//         memory for "data".
		//       The file is binary and has the format described in the specs.


        // open the file with binary mode
        std::ifstream file(filename, std::ios::in | std::ios::binary);

        // check if the file is opened
        if (file) { 
            // read the first int data as total_items
            file.read(reinterpret_cast<char*>(&total_items), sizeof(total_items));
            // allocate memory space to hold the data items
            data = new int[total_items];

            // read the data items into the allocated memory space
            for (int i = 0; i < total_items; i++) {
                file.read(reinterpret_cast<char*>(&data[i]), sizeof(int));
            }
        }
        else {
            // throw an exception if the file cannot be opened
            throw std::string("Cannot open file");
        }

        // close the file
        file.close();

		std::cout << "Item's count in file '"<< filename << "': " << total_items << std::endl;
		std::cout << "  [" << data[0] << ", " << data[1] << ", " << data[2] << ", ... , "
		          << data[total_items - 3] << ", " << data[total_items - 2] << ", "
		          << data[total_items - 1] << "]" << std::endl;

		// Following statements initialize the variables added for multi-threaded 
		//   computation
		num_threads = n_threads; 
		averages = new double[num_threads] {};
		variances = new double[num_threads] {};
		p_indices = new int[num_threads+1] {};
		for (int i = 0; i < num_threads; i++)
			p_indices[i] = i * (total_items / num_threads);
		p_indices[num_threads] = total_items;
	}
	ProcessData::~ProcessData() {
		delete[] data;
		delete[] averages;
		delete[] variances;
		delete[] p_indices;
	}

	ProcessData::operator bool() const {
		return total_items > 0 && data;
	}

	// TODO Improve operator() function from part-1 for multi-threaded operation. Enhance the  
	//   function logic for the computation of average and variance by running the 
	//   function computeAvgFactor and computeVarFactor in multile threads. 
	// The function divides the data into a number of parts, where the number of parts is 
	//   equal to the number of threads. Use multi-threading to compute average-factor for 
	//   each part of the data by calling the function computeAvgFactor. Add the obtained 
	//   average-factors to compute total average. Use the resulting total average as the 
	//   average value argument for the function computeVarFactor, to compute variance-factors 
	//   for each part of the data. Use multi-threading to compute variance-factor for each 
	//   part of the data. Add computed variance-factors to obtain total variance.
	// Save the data into a file with filename held by the argument fname_target. 
	// Also, read the workshop instruction.


	// TODO You create implementation of function operator(). See workshop instructions for details.
    int ProcessData::operator()(const std::string& filename, double& avg, double& var) {
        // create a vector of avg threads
        std::vector<std::thread> avgThreads;
        // create a vector of var threads
        std::vector<std::thread> varThreads;

        std::vector<double> avgVec(num_threads);
        std::vector<double> varVec(num_threads);

        // bind total_items to the function computeAvgFactor as the divisor parameter.
        auto avgBind = std::bind(computeAvgFactor, std::placeholders::_1, std::placeholders::_2, total_items, std::placeholders::_3);

        for (auto i = 0; i < num_threads; i++) {
            avgThreads.push_back(std::thread(avgBind, &data[p_indices[i]], p_indices[i + 1] - p_indices[i], std::ref(avgVec[i])));
        }

        for (auto& thread : avgThreads) {
            // join the threads
            thread.join();
        }

        for (auto i : avgVec) {
            avg += i;
        }

        // bind total_items and avg to the function computeVarFactor as the divisor and avg parameters.
        auto varBind = std::bind(computeVarFactor, std::placeholders::_1, std::placeholders::_2, total_items, avg, std::placeholders::_3);

        for (auto i = 0; i < num_threads; i++) {
            varThreads.push_back(std::thread(varBind, &data[p_indices[i]], p_indices[i + 1] - p_indices[i], std::ref(varVec[i])));
        }

        for (auto& thread : varThreads) {
            // join the threads
            thread.join();
        }

        for (auto i : varVec) {
            var += i;
        }

        // open the file with binary mode
        std::ofstream file(filename, std::ios::out | std::ios::binary);

        // check if the file is opened
        if (file) {
            // write the total_items into the file
            file.write(reinterpret_cast<char*>(&total_items), sizeof(total_items));
            
            // write the data items into the file
            for (int i = 0; i < total_items; i++) {
                file.write(reinterpret_cast<char*>(&data[i]), sizeof(int));
            }
        } else {
            // throw an exception if the file cannot be opened
            throw std::string("Cannot open file");
        }

        // close the file
        file.close();

        // return the total_items (the return value wasn't specified in the instructions)
        return 0;
    }
}