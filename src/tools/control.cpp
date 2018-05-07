
#include <map>

#include <getopt.h>

#include <openservo.h>

using namespace std;
using namespace openservo;

#define CMD_OPTIONS "hv"

void print_help() {

    cout << "Scan for OpenServo devices and print their properties" << endl << endl;

    cout << "\n\nProgram arguments: \n";
    cout << "\t-h\tPrint this help and exit\n";
    cout << "\t-v\tVerbose output\n";
    cout << "\t-l\tSet i2c device location\n";

    cout << "\n";
}

void print_status(ServoHandler s, bool verbose) {

	cout << "   - Address: " << s->getAddress() << " (0x" << hex << s->getAddress() << ")" << dec << endl;
	cout << "   - Type: " << s->getType() << " / " << s->getSubType() << endl;		

	pair<int, int> version = s->getVersion();
	cout << "   - Version: " << version.first << "." << version.second << endl;
	cout << "   - Position: " << s->getPosition() << endl;
	cout << "   - Seek: " << s->getSeekPosition() << endl;
	cout << "   - Min: " << s->getMinSeek() << endl;
	cout << "   - Max: " << s->getMaxSeek() << endl;

	if (verbose) {

		cout << "   - Flags: " << s->getFlags() << endl;
		cout << "   - Timer: " << s->getTimer() << endl;
		cout << "   - Power: " << s->getPower() << endl;
		cout << "   - PWM: " << s->getPWM() << endl;

	}

}

int main(int argc, char** argv) {

	ServoBus bus;
	
	bool verbose = false;
	string location;
	int c;

	int device_address = -1;
	
	std::map<string, int> updates;


	while ((c = getopt(argc, argv, CMD_OPTIONS)) != -1)
	    switch (c) {
	    case 'h':
	        print_help();
	        exit(0);
	    case 'v':
	        verbose = true;
	        break;
	    case 'l':
	        location = string(optarg);
	        break;
	    default:
	        print_help();
	        throw std::runtime_error(string("Unknown switch -") + string(1, (char) optopt));
	    }

	if (optind < argc) { 
		device_address = atoi(argv[optind]);
		optind++;
	}

	while (optind < argc) { 
		char* var = argv[optind];
        char* ptr = strchr(var, '=');
        if (!ptr) break;
        updates[string(var, ptr-var)] = atoi(ptr+1);
		optind++;
	}

	if (!bus.open(location)) {
		cout << "Unable to connect to i2c bus" << endl;
		return -1;
	}

	cout << "Scanning for OpenServo devices ... " << endl;

	int n = bus.scan();

	if (device_address < 0) {

		if (n == 0) {
			cout << "Found no servos" << endl;
			return 0;
		}

		cout << "Found " << n << " servos" << endl;

		for (int i = 0; i < bus.size(); i++) {

			ServoHandler s = bus.get(i);

			cout << " * Servo " << i << endl;

			print_status(s, verbose);

		}

	} else {

		if (!bus.exists(device_address)) {
			cout << "Device does not exist" << endl;
			return -2;
		}

		ServoHandler s = bus.find(device_address);

		if (updates.size() == 0) {

			print_status(s, verbose);

			std::vector<string> registers = s->list();

			cout << "Registers: ";

			for (std::vector<string>::iterator it = registers.begin(); it != registers.end(); it++) {

				if (!s->isReadonly(*it))
					cout << *it << ", ";

			}

			cout << endl;

		} else {

			for(std::map<string, int>::const_iterator it = updates.begin();
    			it != updates.end(); ++it) {

				if (!s->exists(it->first)) {
					cout << "Register does not exist" << endl;
					return -2;
				}

				if (s->isReadonly(it->first)) {
					cout << "Register is readonly" << endl;
					return -2;
				}

				if (s->isProtected(it->first)) {
					cout << "Register is protected" << endl;
					return -2;
				}

				s->set(it->first, it->second);

			}

			bus.update();

		}




	}

}

