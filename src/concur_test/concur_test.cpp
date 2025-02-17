
#include <iostream>
#include <functional>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <Eigen/Dense>
#include <deque>


//class declaration
class Worker
{
public:
	Worker(Eigen::MatrixXd &_mat, Eigen::MatrixXd &_mat2, std::map<int, std::vector<int>> &_cases, std::map<int,Eigen::MatrixXd> &_results);
	Eigen::MatrixXd calc(int id, int col_idx, std::vector<int> row_idxs);
	std::pair<int, std::vector<int>> get_next();

private:
	Eigen::MatrixXd &mat; //by ref so shared between all threads
	Eigen::MatrixXd &mat2;
	std::map<int, std::vector<int>> &cases; // by ref - shared between all threads
	std::map<int, Eigen::MatrixXd> &results;
	Eigen::MatrixXd extract(int col_idx, std::vector<int> row_idxs);
	void put(int col_idx,Eigen::MatrixXd &matrix);
	std::mutex next_lock, extract_lock, put_lock;
	Eigen::MatrixXd emat1, emat2;
	Eigen::MatrixXd build_mat(std::vector<int> row_idxs, Eigen::MatrixXd &extract_from);
	void set_components(int id, std::vector<int> row_idxs);
	std::mutex emat1_lock, emat2_lock;
};


//class construtor
Worker::Worker(Eigen::MatrixXd &_mat, Eigen::MatrixXd &_mat2,std::map<int, std::vector<int>> &_cases, std::map<int,Eigen::MatrixXd> &_results):
	mat(_mat), mat2(_mat2), cases(_cases), results(_results)
{

}


//do some calculations on a subset of the whole matrix
Eigen::MatrixXd Worker::calc(int id, int col_idx, std::vector<int> row_idxs)
{
	set_components(id, row_idxs);
	Eigen::MatrixXd extract_mat = extract(col_idx, row_idxs);
	//some operation
	Eigen::MatrixXd noise = Eigen::MatrixXd::Random(100,100);
	
	extract_mat = extract_mat.array() + 1.0;
	
	Eigen::JacobiSVD<Eigen::MatrixXd> svd(noise, Eigen::ComputeThinU | Eigen::ComputeThinV);
	Eigen::MatrixXd sv = svd.singularValues();

	put(col_idx, sv);
	return extract_mat;
}


//thread safe: put the results into the container
void Worker::put(int col_idx,Eigen::MatrixXd &matrix)
{
	std::lock_guard<std::mutex> g(put_lock);
	if (results.find(col_idx) != results.end())
	{
		std::cout << "shits busted" << std::endl;
	}
	results[col_idx] = matrix;
	return;

}

//not thread safe, so only be called within a locked section

Eigen::MatrixXd Worker::build_mat(std::vector<int> row_idxs, Eigen::MatrixXd &extract_from)
{

	Eigen::MatrixXd extract_to(row_idxs.size(), extract_from.cols());
	std::cout << extract_to.rows() << ", " << extract_to.cols() << std::endl;
	int i = 0;
	for (auto ri : row_idxs)
	{
		extract_to.row(i) = extract_from.row(ri);
		++i;
	}
	return extract_to;
}

//thread safe: let the threads async compete to get the components set that they need
void Worker::set_components(int id, std::vector<int> row_idxs)
{
	emat1.resize(0, 0);
	emat2.resize(0, 0);
	std::unique_lock<std::mutex> emat1_guard(emat1_lock, std::defer_lock);
	std::unique_lock<std::mutex> emat2_guard(emat2_lock, std::defer_lock);
	while (true)
	{
		if ((emat1.rows() > 0) && (emat2.rows() > 0))
			break;
		if ((emat1.rows() == 0) && (emat1_guard.try_lock()))
		{
			emat1 = build_mat(row_idxs, mat);
			
			std::cout << "id: " << id << " , emat1" << std::endl;
			emat1_guard.unlock();
		}
		if ((emat2.rows() == 0) && (emat2_guard.try_lock()))
		{
			emat2 = build_mat(row_idxs, mat2);
			
			std::cout << "id: " << id << " , emat2" << std::endl;
			emat2_guard.unlock();
		}
	}
}

//thread safe: extract the piece we need
Eigen::MatrixXd Worker::extract(int col_idx, std::vector<int> row_idxs)
{

	std::lock_guard<std::mutex> g(extract_lock);
	Eigen::MatrixXd extract_mat(row_idxs.size(), 1);
	int i = 0;
	for (auto ri : row_idxs)
	{
		extract_mat(i, 0) = mat(ri, col_idx);
		++i;
	}
	return extract_mat;
}


//thread safe: get the next par to process
std::pair<int, std::vector<int>> Worker::get_next()
{
	std::lock_guard<std::mutex> g(next_lock);
	//the end condition
	if (cases.size() == 0)
	{
		std::pair<int, std::vector<int>> p(-999,std::vector<int>());
		return p;
	}
	else
	{
		std::map<int, std::vector<int>>::iterator it = cases.begin();
		std::pair<int, std::vector<int>> p(it->first, it->second);
		cases.erase(it);
		return p;
	}
}

//threaded function
void worker_calc(int id, Worker &worker)
{
	while (true)
	{
		std::pair<int, std::vector<int>> p = worker.get_next();
		if ((p.first < 0) || (p.second.size() == 0))
			//no more work to do, so return
			break;
		worker.calc(id, p.first, p.second);
		//not threaded so output can be all jacked up
		std::cout << "id: " << id << ", case: " << p.first << std::endl;
	}
}


//main function
int main()
{
	// matrix dimensions
	int n = 100;
	
	Eigen::MatrixXd mat(n, n);
	mat.setOnes();

	Eigen::MatrixXd mat2 = Eigen::MatrixXd::Random(n, n);
	


	//build up some fake tasks to do 
	std::map<int, std::vector<int>> cases;
	for (int i = 1; i < n;i++)
	{
		std::vector<int> idxs;
		for (int ii = 0; ii < i; ii++)
			idxs.push_back(ii);
		cases[i] = idxs;
	}
	std::map<int, Eigen::MatrixXd> results;
	//the worker with the appropriate thread machinery
	Worker worker(mat, mat2, cases, results);
	//std::pair<int,std::vector<int>> p = worker.get_next();
	//worker.calc(p.first,p.second);
	//start the threads
	std::vector<std::thread> threads;
	int num_threads = 10;
	for (int i = 0; i < num_threads; i++)	
		threads.push_back(std::thread(worker_calc, i, std::ref(worker)));
	
	//join the threads back - this waits until all threads are done
	for (auto &t : threads)
		t.join();

	//check that the results are correct
	/*for (auto r : results)
	{
		std::cout << r.first << ", " << r.second.size() << std::endl;
	}*/
    return 0;
}

