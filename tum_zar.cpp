#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <dirent.h>
#include <stdio.h>

const static int SIZE_CHUNK = 800000;
const static int SIZE_NAME = 200;
static uint32_t primes[200] = {
	2,3,5,7,11,13,17,19,23,29,
	31,37,41,43,47,53,59,61,67,71,
	73,79,83,89,97,101,103,107,109,113,
	127,131,137,139,149,151,157,163,167,173,
	179,181,191,193,197,199,211,223,227,229,
	233,239,241,251,257,263,269,271,277,281,
    283,293,307,311,313,317,331,337,347,349,
    353,359,367,373,379,383,389,397,401,409,
    419,421,431,433,439,443,449,457,461,463,
    467,479,487,491,499,503,509,521,523,541,
    547,557,563,569,571,577,587,593,599,601,
    607,613,617,619,631,641,643,647,653,659,
    661,673,677,683,691,701,709,719,727,733,
    739,743,751,757,761,769,773,787,797,809,
    811,821,823,827,829,839,853,857,859,863,
    877,881,883,887,907,911,919,929,937,941,
    947,953,967,971,977,983,991,997,1009,1013,
   1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,
   1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,
   1153,1163,1171,1181,1187,1193,1201,1213,1217,1223
};

struct stat results;
uint32_t get_size(std::string path_file) {

	if (stat(path_file.c_str(), &results) != 0) {
		std::cout << "error: file not found " << path_file << std::endl;
		return 0;
	}

	return results.st_size;
} 

static std::ifstream read_file;
void read_binary(std::string path_file, uint32_t size, char* buffer, int status = -1) {

	switch (status) {
		case -1:
			read_file.open(path_file);
			read_file.read(buffer, size);
			read_file.close();
		break;
		case 0:
			read_file.open(path_file);
			read_file.read(buffer, size);
		break;
		case 1:
			read_file.read(buffer, size);
		break;
		case 2:
			read_file.read(buffer, size);
			read_file.close();
		break;
	}

}

static std::ofstream write_file;
void write_binary(std::string path_file, uint32_t size, char* buffer, int status = -1) {

	switch (status) {
		case -1:
			write_file.open(path_file);
			write_file.write(buffer, size);
			write_file.close();
		break;
		case 0:
			write_file.open(path_file);
			write_file.write(buffer, size);
		break;
		case 1:
			write_file.write(buffer, size);
		break;
		case 2:
			write_file.write(buffer, size);
			write_file.close();
		break;
	}

}

std::string get_unique_name(std::string output_path, int& n) {
	std::string output_path_file;
	bool exists = true;
	while (exists) {

		output_path_file = output_path + std::to_string(n++) + ".zar";
		read_file.open(output_path_file);
		exists = read_file.good();
		read_file.close();

	}

	return output_path_file;
}

void encrypt_buffer(char* buffer, uint32_t size, std::string secret_key) {
	uint8_t len = secret_key.length();
	uint8_t key_quotient = 1;
	for (uint8_t i = 0; i < len; i++) {
		if ((uint8_t)secret_key[ i ] >= 200) {
			std::cout << "Error: character " << secret_key[ i ] << " not supported" << std::endl;
		}
		key_quotient *= primes[secret_key[ i ]];
	}

	uint8_t id_secret_key = 0;
	uint32_t id_buffer = 0;
	while (id_buffer < size) {
		uint8_t k = secret_key[ id_secret_key ];

		buffer[ id_buffer ] ^= (k * key_quotient);

		id_secret_key = id_secret_key >= len ? 0 : id_secret_key + 1;
		id_buffer++;
	}
}

class Path_zar {
public:
	Path_zar(std::string path) {
		this->path = path;
	} 
	std::string get_name() {
		for (uint8_t i = path.length() - 1; i >= 0; i--) {
			if (path[ i ] == '/')
				return path.substr(i + 1);
		} 
		return "";
	}
	std::string get_path() {
		for (uint8_t i = path.length() - 1; i >= 0; i--) {
			if (path[ i ] == '/')
				return path.substr(0, i + 1);
		} 
		return "";
	}
	std::string get_extension() {
		for (uint8_t i = path.length() - 1; i >= 0; i--) {
			if (path[ i ] == '.')
				return path.substr(i + 1);
		} 
		return "";
	}
	uint8_t get_length() {
		return path.length();
	}
	std::string get() {
		return path;
	}
private:
	std::string path;
};

void process_directory(std::string directory);
void process_file(std::string file);
void process_entity(struct dirent* entity);
static std::string base_path;
static int original_len = 0;
static bool is_just_a_file = false;
static std::vector<std::string> list_task;
void process_directory(std::string directory) {

    std::string dirToOpen = base_path + directory;
    auto dir = opendir(dirToOpen.c_str());

    //set the new path for the content of the directory
    base_path = dirToOpen + "/";

    // std::cout << "Process directory: " << dirToOpen.substr(original_len).c_str() << std::endl;

    if (NULL == dir) {
        is_just_a_file = true;
        return;
    }

    auto entity = readdir(dir);

    while (entity != NULL) {
        process_entity(entity);
        entity = readdir(dir);
    }

    //we finished with the directory so remove it from the path
    base_path.resize(base_path.length() - 1 - directory.length());
    closedir(dir);
}

void process_entity(struct dirent* entity) {

    //find entity type
    if (entity->d_type == DT_DIR) {
    	//it's an direcotry

        //don't process the  '..' and the '.' directories
        if(entity->d_name[0] == '.')
            return;

        //it's an directory so process it
        process_directory(std::string(entity->d_name));
        return;
    }

    if(entity->d_type == DT_REG) {
    	 //don't process the '.' (hidden files)
    	if(entity->d_name[0] == '.')
            return;

    	//regular file
        process_file(std::string(entity->d_name));
        return;
    }

    //there are some other types
    //read here http://linux.die.net/man/3/readdir
    std::cout << "Not a file or directory: " << entity->d_name << std::endl;
}

void process_file(std::string file) {

	std::string file_path = base_path + file;

	//std::string instance_name = file_path.substr(original_len);
	
	//std::cout << "put " << file_path << std::endl;
	list_task.push_back(file_path);

}

std::string get_name_by_buffer(char* buffer, int begin_at) {

	std::string name = "";
	while (buffer[ begin_at ] != 0) {
		name += buffer[ begin_at++ ];
	}

	return name;
}

std::string get_secret_key(std::string secret_key_path) {
	char buffer_secret_key[ 600 ];
	for (uint16_t i = 0; i < 600; i++)
		buffer_secret_key[ i ] = 0;

	uint32_t secret_key_size = get_size(secret_key_path);

	read_file.open(secret_key_path);
	if ( ! read_file.good()) {
		read_file.close();
		return "";
	}

	read_file.read(buffer_secret_key, secret_key_size);
	read_file.close();
	std::remove(secret_key_path.c_str());

	 return get_name_by_buffer(buffer_secret_key, 0);
}

void run_encrypt(std::vector<std::string>& paths, std::string secret_key) {
	
	int n_name_output = 0;

	double progress = 0;
	double total = paths.size();
	for (uint32_t i = 0; i < paths.size(); i++) {
		std::string current_task = paths[ i ];
		Path_zar pathzar(current_task);

		uint32_t size_file = get_size(current_task);
		if (size_file == 0)
			continue;

		uint32_t current_size;
		bool complete_load = false;
		bool first_load = true;
	
		std::string output_path = pathzar.get_path();
		std::string output_path_file = get_unique_name(output_path, n_name_output);

		do {
			if (size_file > SIZE_CHUNK + SIZE_NAME) {
				current_size = SIZE_CHUNK;
			} else {
				current_size = size_file;
				complete_load = true;
			}

			if (complete_load)
				current_size += SIZE_NAME;

			char buffer[ current_size ];
			
			if (first_load && complete_load)
				read_binary(current_task, current_size, buffer, -1);
			else if (first_load)
				read_binary(current_task, current_size, buffer, 0);
			else if (complete_load)
				read_binary(current_task, current_size, buffer, 2);
			else
				read_binary(current_task, current_size, buffer, 1);

			if (complete_load) {
				std::string saved_name = pathzar.get_name();
				for (uint8_t j = 0; j < SIZE_NAME; j++) {
					if (j < saved_name.length())
						buffer[ current_size - SIZE_NAME + j ] = saved_name[ j ];
					else
						buffer[ current_size - SIZE_NAME + j ] = 0;
				}
			}

			encrypt_buffer(buffer, current_size, secret_key);
			
			if (first_load && complete_load)
				write_binary(output_path_file, current_size, buffer, -1);
			else if (first_load)
				write_binary(output_path_file, current_size, buffer, 0);
			else if (complete_load)
				write_binary(output_path_file, current_size, buffer, 2);
			else
				write_binary(output_path_file, current_size, buffer, 1);

			size_file -= SIZE_CHUNK;
			first_load = false;

		} while( ! complete_load);

		std::remove(pathzar.get().c_str());

		progress++;
		double loading = (progress / total) * 100;
		std::cout << loading << " " << current_task << std::endl;
	}
	
}

void run_decrypt(std::vector<std::string>& paths, std::string secret_key) {

	double progress = 0;
	double total = paths.size();
	for (uint32_t i = 0; i < paths.size(); i++) {
		std::string current_task = paths[ i ];
		Path_zar pathzar(current_task);

		uint32_t size_file = get_size(current_task);
		if (size_file == 0)
			continue;

		uint32_t current_size;
		bool complete_load = false;
		bool first_load = true;

		std::string output_path_file = pathzar.get_path();
		output_path_file += "tum_zar_temp";
		do {

			if (size_file > SIZE_CHUNK + SIZE_NAME + SIZE_NAME) {
				current_size = SIZE_CHUNK;
			} else {
				current_size = size_file;
				complete_load = true;
			}

			char buffer[ current_size ];
			if (first_load && complete_load)
				read_binary(current_task, current_size, buffer, -1);
			else if (first_load)
				read_binary(current_task, current_size, buffer, 0);
			else if (complete_load)
				read_binary(current_task, current_size, buffer, 2);
			else
				read_binary(current_task, current_size, buffer, 1);

			encrypt_buffer(buffer, current_size, secret_key);

			std::string renamed_output;
			if (complete_load) {
				std::string original_name = get_name_by_buffer(buffer, current_size - SIZE_NAME);
				renamed_output = pathzar.get_path() + original_name;
			}

			if (first_load && complete_load) {
				write_binary(renamed_output, current_size - SIZE_NAME, buffer, - 1);
			} else if (first_load) {
				write_binary(output_path_file, current_size, buffer, 0);
			} else if (complete_load) {
				write_binary(output_path_file, current_size - SIZE_NAME, buffer, 2);
				if (std::rename(output_path_file.c_str(), renamed_output.c_str()) != 0)
					std::cout << "error renaming file: " << renamed_output << std::endl;
			} else {
				write_binary(output_path_file, current_size, buffer, 1);					
			}

			size_file -= SIZE_CHUNK;
			first_load = false;
		} while ( ! complete_load);

		std::remove(pathzar.get().c_str());

		progress++;
		double loading = (progress / total) * 100;
		std::cout << loading << " " << current_task << std::endl;
	}	

}

int main(int argc, char** argv) {

	if (argc <= 3) {
		std::cout << "Tum_zar\narg[ 0 ] path origin\narg[ 1 ] enc / dec\narg[ 2 ] secret key path" << std::endl;
		return 0;
	}

	std::string path_file = argv[ 1 ];
	std::string command = argv[ 2 ];
	std::string secret_key_path = argv[ 3 ];

	if (command != "enc" && command != "dec") {
		std::cout << "arg[ 2 ] " << command << " not valid (enc / dec)" << std::endl;
		return 0;
	}
	
	std::string secret_key = get_secret_key(secret_key_path);

	if (secret_key == "") {
		std::cout << "arg[ 3 ] " << secret_key_path << " is not a valid path to read a secret key" << std::endl;
		return 0;
	}

	process_directory(path_file);

	if (is_just_a_file) {
		list_task.push_back(path_file);
	}

	if (list_task.size() == 0) {
		std::cout << "Not found files" << std::endl;
		return 0;
	}

	std::vector<std::string> designated_files;
	if (command == "enc") {

		for (uint32_t i = 0; i < list_task.size(); i++) {
			Path_zar pathzar(list_task[ i ]);
			if (pathzar.get_extension() != "zar")
				designated_files.push_back(list_task[ i ]);
		}

		run_encrypt(designated_files, secret_key);

	} else {

		for (uint32_t i = 0; i < list_task.size(); i++) {
			Path_zar pathzar(list_task[ i ]);
			if (pathzar.get_extension() == "zar")
				designated_files.push_back(list_task[ i ]);
		}

		run_decrypt(designated_files, secret_key);

	}
	
	
	return 0;
}















