#ifndef UTILITIES_H_
#define UTILITIES_H_

/* @file
 @brief Utility Functions

 This file contains a variety of utility functions for string and numeric operations.
*/

#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <exception>
#include "pest_error.h"
#include "Transformable.h"
#include "network_package.h"
#include <thread>
#include <chrono>
#include <Eigen/Dense>
#include <Eigen/Sparse>




//printing functions
std::ostream& operator<< (std::ostream &os, const std::set<std::string> val);
std::ostream& operator<< (std::ostream &os, const std::vector<std::string> val);
void print(std::set<std::string> val, std::ostream &os, int indent=0);

namespace pest_utils
{

	enum CASE_CONV{NO_CONV, TO_UPPER, TO_LOWER};

	double get_duration_sec(std::chrono::system_clock::time_point start_time);

   /* @brief Sign of a number

   Returns the sign of a val (-1, 1 or 0).
  */
	inline int sign(double val)
	{
		if (val > 0) return 1;
		if (val < 0) return -1;
		return 0;
	}


	/* @brief Splits a string and returns the sub-strings.

	String str is split into sub-strings using the characters specified
	in delimiters and the sub-strings are are returned in the tokens constainer.
	If trimEmpty is specified as true, empty records are removed from
	tokens before it is returned
	*/
	template < class ContainerT >
	void tokenize(const std::string& str, ContainerT& tokens,
		const std::string& delimiters=" \t\n\r", const bool trimEmpty=true);


	/* @brief Convert a string to another type.

	String s is converted to the type of the return variable x.  If extra
	characters are left over after the conversion and failIfLeftoverChars is set
	to true, a PestConversionError exception is thrown
	*/
	template<typename T>
	inline void convert_ip(string const &s, T& x, bool failIfLeftoverChars = true)
	{
		istringstream i(s);
		char c;
		if (!(i >> x) || (failIfLeftoverChars && i.get(c))) {
			throw PestConversionError(s);
		}
	}

	/* @brief Convert a string to another type.

	String s is converted to the type T.  If extra
	characters are left over after the conversion and failIfLeftoverChars is set
	to true, a PestConversionError exception is thrown.

	The tepmplatized return type must be included in the fuction call.  The following example shows
	how to call this function to convert a string to an integer
	convert_cp<int>("10")
	*/
	template<typename T>
	inline T convert_cp(std::string const& s,
		bool failIfLeftoverChars = true)
	{
		T x;
		convert_ip(s, x, failIfLeftoverChars);
		return x;
	}

	/* @brief Strip leading and/or trailing characters from a string

		The characters contained in arguement delimiters are stripped from
		string s.  op can be specified as "front", "back" or "both" to control
		whether the characters are stripped from the begining, end or both sides
		of string s.
	*/



	std::string& strip_ip(string &s, const string &op="both",
		const string &delimiters=" \t\n\r");

	/* @brief Strip leading and/or trailing characters from a string

		The characters contained in arguement delimiters are stripped from
		string s and the updated string is returned without modifying s.  op can be specified as "front", "back" or "both" to control
		whether the characters are stripped from the begining, end or both sides
		of string s.
	*/
	string strip_cp(const string &s, const string &op="both",
		const string &delimiters=" \t\n\r");

	/* @brief Convert all the characters in a string to upper case

		All characters in string s are converted to upper case
	*/
	void upper_ip(string &s);

	/* @brief Convert all the characters in a string to upper case

		All characters in string s are converted to upper case and returned as
		a new string without modifying the original string
	*/
	string upper_cp(const string &s);

		/* @brief Convert all the characters in a string to upper case.

		All characters in string s are converted to lower case.
	*/
	string upper(char *);

	void lower_ip(string &s);

	/* @brief Convert all the characters in a string to lower case.

		All characters in string s are converted to lower case and returned as
		a new string without modifying the original string.
	*/
	string lower_cp(const string &s);

	/* @brief Return the base filename (filename without the "." extention).
	*/
	string get_base_filename(const string &s);

	/* @brief Converts a C++ string to a FORTRAN character array.

		Converts string "in" to a FORTRAN character array of length "length".
	*/
	void string_to_fortran_char(string in, char out[], int length, CASE_CONV conv_type=NO_CONV);

	vector<char> string_as_fortran_char_ptr(string in, int length);


	string get_filename_without_ext(const string &filename);
	/* @brief Given a combined path and filename return just the filename.

		Given path the combined path and filname complete_path, return just the filename.
	*/
	string get_filename_ext(const string &filename);

	string get_filename(const string &complete_path);

	/* @brief Given a combined path and filename return just the pathname.

		Given path the combined path and filname complete_path, return just the pathname.
	*/
	string get_pathname(const string &complete_path);


	template <class keyType, class dataType>
	vector<keyType> get_map_keys(const map<keyType,dataType> &my_map);

class String2CharPtr
{
public:
	String2CharPtr(const std::string &str);
	char *get_char_ptr();
	~String2CharPtr() {}
private:
	std::vector<char> my_str;
};


class StringvecFortranCharArray
{
public:
	StringvecFortranCharArray(const vector<string> in, int length, CASE_CONV conv_type=NO_CONV );
	char *get_prt();
	~StringvecFortranCharArray();
private:
	char *fort_array;

};

template <class type>
class CompareItemInSet
{
	public:
		CompareItemInSet(const std::set<type> &_set_ref) : set_ref(_set_ref){};
		bool operator()(const type &item) {return set_ref.count(item) > 0;}
		~CompareItemInSet(){}
	private:
		const std::set<type> &set_ref;
};



void copyfile(const string &from_file, const string &to_file);

std::string fortran_str_2_string(char *fstr, int str_len);

std::vector<std::string> fortran_str_array_2_vec(char *fstr, int str_len, int fstr_len);

void read_par(ifstream &fin, Parameters &pars);

void read_res(string &res_filename, Observations &obs);

bool check_exist_in(std::string filename);

bool check_exist_out(std::string filename);

//template <class dataType>
//void read_twocol_ascii_to_map(std::map<std::string, dataType> &result,std::string filename, int header_lines=0, int data_col=1);

map<string, double> read_twocol_ascii_to_map(std::string filename, int header_lines = 0, int data_col = 1);

class thread_flag
{
public:

	thread_flag(bool _flag);
	bool set(bool _flag);
	bool get();

private:
	bool flag;
	std::mutex m;

};

class thread_exceptions
{
public:

	thread_exceptions() {}
	void add(std::exception_ptr ex_ptr);
	void rethrow();
	int size(){ return shared_exception_vec.size(); }

private:
	std::vector<std::exception_ptr> shared_exception_vec;
	std::mutex m;

};

class thread_RAII
{
	thread& t;
public:
	thread_RAII(thread& th) :t(th)
	{
	}

	~thread_RAII()
	{
		if (t.joinable())
		{
			t.join();
		}
	}

private:
	// copy constructor
	thread_RAII(const thread_RAII& thr);

	// copy-assignment operator
	thread_RAII& operator=(const thread_RAII& thr);
};


bool read_binary(const string &filename, vector<string> &row_names, vector<string> &col_names, Eigen::SparseMatrix<double> &matrix);

bool read_binary(const string &filename, vector<string> &row_names, vector<string> &col_names, Eigen::MatrixXd &matrix);

}  // end namespace pest_utils
#endif /* UTILITIES_H_ */
