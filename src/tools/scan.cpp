
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

    cout << "\n";
}

int main(int argc, char** argv) {

	ServoBus bus;
	
	bool verbose = false;
	string location;
	int c;

	while ((c = getopt(argc, argv, CMD_OPTIONS)) != -1)
	    switch (c) {
	    case 'h':
	        print_help();
	        exit(0);
	    case 'v':
	        verbose = true;
	        break;
	    default:
	        print_help();
	        throw std::runtime_error(string("Unknown switch -") + string(1, (char) optopt));
	    }

	if (optind < argc) { 
		location = string(argv[optind]);
	}

	if (!bus.open(location)) {
		cout << "Unable to connect to i2c bus" << endl;
		return -1;
	}

	cout << "Scanning for OpenServo devices ... " << endl;

	int n = bus.scan();

	if (n == 0) {
		cout << "Found no servos" << endl;
		return 0;
	}

	cout << "Found " << n << " servos" << endl;

	for (int i = 0; i < bus.size(); i++) {

		ServoHandler s = bus.get(i);

		cout << " * Servo " << i << endl;

		cout << "   - Address: " << s->getAddress() << " (0x" << hex << s->getAddress() << ")" << endl;
		cout << "   - Type: " << s->getType() << " / " << s->getSubType() << endl;		

		cout << "   - Version: " << s->getVersion() << endl;
		cout << "   - Position: " << s->getPosition() << endl;
		cout << "   - Seek: " << s->getSeekPosition() << endl;

		if (verbose) {

			cout << "   - Flags: " << s->getFlags() << endl;
			cout << "   - Timer: " << s->getTimer() << endl;
			cout << "   - Power: " << s->getPower() << endl;
			cout << "   - PWM: " << s->getPWM() << endl;

		}

	}

}

