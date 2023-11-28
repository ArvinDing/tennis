#include <bits/stdc++.h>

using namespace std;
const int TROOPS = 100;
bool win[TROOPS + 1][TROOPS + 1][5] { };
int bestChoice[TROOPS + 1][TROOPS + 1][5] { };
bool isWin(int yourTroops, int oppTroops, int score) {
	if (score >= 3)
		return true;
	if (score <= -3)
		return false;
	return win[yourTroops][oppTroops][score + 2];
}

int main() {
	freopen("forced.json", "w", stdout);
	cout << "[";
	bool first = true;
	for (int yourTroops = 0; yourTroops <= TROOPS; yourTroops++) {
		for (int oppTroops = 0; oppTroops <= TROOPS; oppTroops++) {
			for (int score = -2; score <= 2; score++) {
				for (int choose = 1; choose <= yourTroops; choose++) {
					bool good = true;
					for (int oppChoice = 0; oppChoice <= oppTroops;
							oppChoice++) {
						int scoreDiff = 0;
						if (choose < oppChoice)
							scoreDiff--;
						if (choose > oppChoice)
							scoreDiff++;
						if (!isWin(yourTroops - choose, oppTroops - oppChoice,
								score + scoreDiff)) {
							good = false;
						}
					}
					if (good) {
						if (!first) {
							cout << ",";
						}
						first=false;
						cout << "{\"state\": [";
						cout << yourTroops << "," << oppTroops << "," << score
								<< "],\n";
						cout << "\"choice\": " << choose << "}\n";
						win[yourTroops][oppTroops][score + 2] = true;
						bestChoice[yourTroops][oppTroops][score + 2] = choose;
						break;
					}
				}
			}
		}
	}
	cout << "]";
}
