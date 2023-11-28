#pragma GCC optimize ("O3")

#include <bits/stdc++.h>

using namespace std;

const int TROOPS = 50;
const int TRAINITER = 500;
const double ALPHA = 0.5;

class Player {
protected:
public:

	virtual ~Player() {
	}
	virtual int play(int soldiers, int oppSoldiers, int score) =0;
};

class RandomPlayer: public Player {
public:
	RandomPlayer() {

	}
	int play(int soldiers, int oppSoldiers, int score) override {
		int use = rand() % (soldiers + 1);
		return use;
	}
};

class Game {
	shared_ptr<Player> a;
	shared_ptr<Player> b;
	int aSoldiers = TROOPS;
	int bSoldiers = TROOPS;
	int score = 0;
public:
	Game(shared_ptr<Player> a, shared_ptr<Player> b) :
			a(a), b(b) {
	}
	//1 if a wins,0 tie, -1 if b wins
	int play() {
		while (aSoldiers || bSoldiers) {
			int aTurn = a->play(aSoldiers, bSoldiers, score);
			int bTurn = b->play(bSoldiers, aSoldiers, -score);
			if (aTurn > bTurn) {
				score++;
			}
			if (aTurn < bTurn) {
				score--;
			}
			if (score >= 3 || score <= -3)
				break;
		}
		return score / 3;
	}
};
bool print = false;
class perfectPlayer: Player {

	double highVal[TROOPS + 1][TROOPS + 1][5] { };
	//highest val achieved against worst counter
	//+1 for winning, 0 for draw, -1 for losing

	vector<double> dist[TROOPS + 1][TROOPS + 1][5];
	//[score, opp score, {-2,-1,0,1,2} game score] = logits

	//make logits sum up to 0
	void normLogits(int yourTroops, int oppTroops, int score) {
		vector<double> &pdf = dist[yourTroops][oppTroops][score + 2];
		double sum = 0;
		for (auto a : pdf)
			sum += a;
		for (auto &a : pdf)
			a -= sum / pdf.size();
	}

	double getVal(int yourTroops, int oppTroops, int score) {
		if (score == 3 || score == -3)
			return score / 3;
		if (yourTroops == 0 && oppTroops == 0)
			return 0;
		assert(2 >= score && score >= -2);
		return highVal[yourTroops][oppTroops][score + 2];
	}

	double sumExp(int yourTroops, int oppTroops, int score) {
		assert(
				(int )dist[yourTroops][oppTroops][score + 2].size()
						== yourTroops + 1);
		double sum = 0;
		for (double d : dist[yourTroops][oppTroops][score + 2]) {
			sum += exp(d);
		}
		return sum;
	}

	vector<double> getPDF(int yourTroops, int oppTroops, int score) {
		vector<double> pdf = dist[yourTroops][oppTroops][score + 2];
		double t = sumExp(yourTroops, oppTroops, score);
		for (auto &a : pdf)
			a = exp(a) / t;
		return pdf;
	}

	//best counter to current dist(ignore (0,0) case)
	//assume all cases under are done
	int counter(int yourTroops, int oppTroops, int score) {

		vector<double> pdf = getPDF(yourTroops, oppTroops, score);
		vector<double> coeff(oppTroops + 1, 0);
		for (int yT = 0; yT <= yourTroops; yT++) {
			for (int oT = 0; oT <= oppTroops; oT++) {
				double probChoose = pdf[yT];
				int scoreDiff = 0;
				if (yT < oT) {
					scoreDiff--;
				} else if (oT < yT) {
					scoreDiff++;
				}
				if (yT == 0 && oT == 0)
					continue;

				coeff[oT] += probChoose
						* getVal(yourTroops - yT, oppTroops - oT,
								score + scoreDiff);
			}
		}
		return std::distance(coeff.begin(),
				std::min_element(coeff.begin(), coeff.end()));
	}

	//take counter and adjust by gradient times learning rate(alpha)
	void adjust(int yourTroops, int oppTroops, int score) {
		vector<double> pdf = getPDF(yourTroops, oppTroops, score);
		int choice = counter(yourTroops, oppTroops, score);
		vector<double> gradient(yourTroops + 1, 0);
		for (int troops = 0; troops <= yourTroops; troops++) {
			int scoreDiff = 0;
			if (choice < troops) {
				scoreDiff = +1;
			} else if (choice > troops) {
				scoreDiff = -1;
			}
			gradient[troops] += pdf[troops]
					* getVal(yourTroops - troops, oppTroops - choice,
							score + scoreDiff);	//not the real gradient
		}
		for (int i = 0; i <= yourTroops; i++) {
			dist[yourTroops][oppTroops][score + 2][i] += ALPHA * gradient[i];
		}
		normLogits(yourTroops, oppTroops, score);

	}

	//value of position against best counter
	double bestVal(int yourTroops, int oppTroops, int score) {

		vector<double> pdf = getPDF(yourTroops, oppTroops, score);
		int choice = counter(yourTroops, oppTroops, score);
		double expectedVal = 0;
		for (int troops = 0; troops <= yourTroops; troops++) {
			int scoreDiff = 0;
			if (choice < troops) {
				scoreDiff = +1;
			} else if (choice > troops) {
				scoreDiff = -1;
			}

			expectedVal += pdf[troops]
					* getVal(yourTroops - troops, oppTroops - choice,
							score + scoreDiff);
		}
		return expectedVal;
	}
public:
	//train the player
	void think() {
		//iterative approach
		for (int sumTroops = 0; sumTroops <= 2 * TROOPS; sumTroops++) {
			for (int yourTroops = max(sumTroops - TROOPS, 0);
					yourTroops <= min(TROOPS, sumTroops); yourTroops++) {
				int oppTroops = sumTroops - yourTroops;
				for (int score = -2; score <= 2; score++) {
					vector<double> &logits = dist[yourTroops][oppTroops][score
							+ 2];
					double &pdfScore = highVal[yourTroops][oppTroops][score + 2];
					logits.assign(yourTroops + 1, 0);

					//loop over each state
					for (int iter = 0; iter < TRAINITER; iter++) {
						adjust(yourTroops, oppTroops, score);
					}
					pdfScore = bestVal(yourTroops, oppTroops, score);
				}
			}
		}
	}

	void printThoughts(int yourTroops, int oppTroops, int score) {
		print = true;
		cout << yourTroops << "vs" << oppTroops << " at " << score << "\n";
		cout << "val " << highVal[yourTroops][oppTroops][score + 2] << "\n";
		cout << "pdf\n";
		auto save = getPDF(yourTroops, oppTroops, score);
		for (int i = 0; i < save.size(); i++) {
			cout << "p(" << i << "): " << save[i] << "\n";
		}
	}

	int play(int yourTroops, int oppTroops, int score) {
		double p = ((double) rand() / (RAND_MAX));
		auto save = getPDF(yourTroops, oppTroops, score);
		double cumulative = 0;
		for (int i = 0; i < save.size(); i++) {
			cumulative += save[i];
			if (cumulative >= p) {
				return i;
			}
		}
		return 0;
	}

};

int main() {
	srand(time(NULL));

	shared_ptr<perfectPlayer> smart = make_shared<perfectPlayer>();
	smart->think();
	int botSoldiers = 50;
	int alexSoldiers = 50;
	int score = 0;
	while (true) {
		smart->printThoughts(botSoldiers, alexSoldiers, score);
		int move = smart->play(botSoldiers, alexSoldiers, score);
		cout << move << "\n";
		int alex;
		cin >> alex;
		if (move > alex)
			score++;
		if (move < alex)
			score--;
		alexSoldiers -= alex;
		botSoldiers -= move;
	}

}
