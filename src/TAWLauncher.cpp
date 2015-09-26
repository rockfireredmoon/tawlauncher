/*
 ============================================================================
 Name        : taw-launcher.cpp
 Author      : Emerald Icemoon
 Version     : 0.0.1
 Copyright   : The IceEE team
 Description : A simple application that allows Spark.exe to be launched
 	 	 	   via the game website. It is used to work around the fact
 	 	 	   we cannot make any changes to the player executable, but
 	 	 	   need to accept 'TAW' urls. The current player will choke
 	 	 	   if anything other than a normal http or file URL is used.
 	 	 	   This launcher encodes the authentication tokens into the
 	 	 	   composition URL, which is then decoded in the client
 	 	 	   preloader.
 ============================================================================
 */

#include <iostream>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32)
#else
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#endif

using namespace std;

std::string url_encode(const std::string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << setw(2) << int((unsigned char) c);
    }

    return escaped.str();
}

string url_decode(string &str) {
	string ret;
	char ch;
	int i, ii;
	for (i = 0; i < str.length(); i++) {
		if (int(str[i]) == 37) {
			sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		} else if (str[i] == '+') {
			ret += " ";
		} else {
			ret += str[i];
		}
	}
	return (ret);
}

std::vector<std::string> &split(const std::string &s, char delim,
		std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

std::string dirname(std::string path) {
	char s = '/';
	#if defined(_WIN32) || defined(WIN32)
		s = '\\';
	#endif
	int idx = path.find_last_of(s);
	if(idx >= path.size()) {
		return ".";
	}
	return path.substr(0, idx);
}

void set_cwd(std::string cwd) {
    std::cout << "Setting working directory to " << cwd << "\n";
	#if defined(_WIN32) || defined(WIN32)
	// TODO
	#else
		chdir(cwd.c_str());
	#endif
}

std::string get_cwd() {
	#if defined(_WIN32) || defined(WIN32)
	return "TODO";
	#else
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL)
			return cwd;
		return "";
	#endif
}

bool is_absolute(std::string str) {
	#if defined(_WIN32) || defined(WIN32)
	// TODO
	return false;
	#else
		return str.find("/") == 0;
	#endif
}

int exec_player(std::string player, std::string composition, std::vector<std::string> args) {
	// TODO args
	// TODO mac
	// TODO alternate wine

	#if defined(_WIN32) || defined(WIN32)
	#else
	std::string programPath = "/opt/wine-staging/bin/wine";
	pid_t pid = fork();
	switch (pid) {
	    case -1:
	        std::cerr << "Fork failed.\n";
	        exit(1);
	    case 0:
	        execlp(programPath.c_str(), programPath.c_str(), player.c_str(), composition.c_str(), (char*)NULL); /* Execute the program */
	        std::cerr << "Exec failed.\n";
	        exit(1);
	    default:
	        std::cout << "Process created with pid " << pid << "\n";
	        int status;

	        while (!WIFEXITED(status)) {
	            waitpid(pid, &status, 0);
	        }
	        std::cout << "Process exited with " << WEXITSTATUS(status) << "\n";
	        return 0;
	}
	#endif
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0]
				<< " <tawUL> [--player <playerExePath>]" << std::endl;
		return 1;
	}
	std::vector<std::string> playerArgs;
	std::string exe = argv[0];
	std::string player;
	std::string composition;
	std::string wat;
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--player") {
			if (i + 1 < argc) {
				player = argv[++i];
			} else {
				std::cerr << "--player option requires one argument."
						<< std::endl;
				return 1;
			}
		} else {
			std::string a = argv[i];
			if (a.find("taw://") == 0) {
				// Is a TAW URL
				a = a.substr(6);
				std::vector<std::string> taw = split(a, ':');
				if (taw.size() < 3) {
					std::cerr
							<< "TAW URL should consist of taw://[sessionName]:[sessionId]:[uid]:[composition]"
							<< std::endl;
					return 1;
				} else {
					wat = taw[0] + ":" + taw[1] + ":" + taw[2];
					if (taw.size() > 3) {
						composition = url_decode(taw[3]);
					}
				}
			} else {
				if (composition.size() == 0) {
					composition = a;
				} else
					playerArgs.push_back(argv[i]);
			}
		}
	}

	std::string cwd = get_cwd();
	if(is_absolute(exe)) {
		cwd = dirname(exe);
	}
	else {
		#if defined(_WIN32) || defined(WIN32)
		cwd = cwd + "\\" + dirname(exe);
		#else
		cwd = cwd + "/" + dirname(exe);
		#endif
	}

	if(player.size() == 0) {
		#if defined(_WIN32) || defined(WIN32)
		player = cwd + "\\Spark.exe";
		#else
		player = cwd + "/Spark.exe";
		#endif
	}

	set_cwd(dirname(player));

	if(composition.size() == 0) {
		std::cerr
				<< "Composition not specified"
				<< std::endl;
		return 1;
	}

	cout << "Player:" << player << endl;
	cout << "Composition:" << composition << endl;

	if(wat.size() > 0) {
		composition = composition + "#web_auth_token=" + url_encode(wat);
	}

	exec_player(player, composition, playerArgs);

	return 0;
}
