#include <iostream>
#include <sstream>
#include <cctype>
#include <windows.h> // coloring
#include <stack>
#include <vector>
#include "bigint.h"

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

using namespace std;
using bi = bigint;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
vector<string> before;

void color(int text, int background = 0) {
	SetConsoleTextAttribute(hConsole, text + 16 * background);
	return;
}

void clear() {
	system(CLEAR_COMMAND);
	return;
}

void press_enter() {
	cout << "Press Enter to Continue...";
	cin.get();
	return;
}

// better error handling
class ZeroDivisionError : public runtime_error {
public:
	ZeroDivisionError(const string& msg) : runtime_error(msg) {}
};
class UnknownOperatorError : public runtime_error {
public:
	UnknownOperatorError(const string& msg) : runtime_error(msg) {}
};
class InvalidExpressionError : public runtime_error {
public:
	InvalidExpressionError(const string& msg) : runtime_error(msg) {}
};
class InvalidUnaryMinusError : public runtime_error {
public:
	InvalidUnaryMinusError(const string& msg) : runtime_error(msg) {}
};
class InvalidCharacterError : public runtime_error {
public:
	InvalidCharacterError(const string& msg) : runtime_error(msg) {}
};
class UnbalancedGroupingSymbolsError : public runtime_error {
public:
	UnbalancedGroupingSymbolsError(const string& msg) : runtime_error(msg) {}
};
class InvalidFractionInput : public runtime_error {
public:
	InvalidFractionInput(const string& msg) : runtime_error(msg) {}
};

class fraction {
private:
	bi upper = 1; // numerator
	bi lower = 1; // denominator

public:
	fraction() {};
	fraction(bi a, bi b = 1) {
		upper = a;
		lower = b;
	}

	bi getUpper() const {
		return upper;
	}
	bi getLower() const {
		return lower;
	}
	void setUpper(bi a) {
		upper = a;
		return;
	}
	void setLower(bi b) {
		lower = b;
		return;
	}

	// fractino to string
	string toString() const {
		ostringstream oss;
		oss << upper << "/" << lower;
		return oss.str();
	}

	bi GCD(bi left, bi right) const {
		while (right != 0) {
			bi div = left % right;
			left = right;
			right = div;
		}
		return left;
	}

	bi ABS(bi orig) const {
		if (orig < 0) orig *= -1;
		return orig;
	}

	bool operator==(const fraction& other) const {
		return (this->upper == other.getUpper()) && (this->lower == other.getLower());
	}
	bool operator!=(const fraction& other) const {
		return !(*this == other);
	}

	// a/b + c/d = (ad + bc) / bd
	fraction operator+(const fraction& other) const {
		fraction result;
		result.lower = this->lower * other.getLower();
		result.upper = (this->lower * other.getUpper()) + (this->upper * other.getLower());

		bi cur_gcd = GCD(ABS(result.lower), ABS(result.upper));
		result.lower /= cur_gcd;
		result.upper /= cur_gcd;

		if (result.lower < 0) {
			result.lower *= -1;
			result.upper *= -1;
		}

		return result;
	}

	// a/b - c/d = (ad - bc) / bd
	fraction operator-(const fraction& other) const {
		fraction result;
		result.lower = this->lower * other.getLower();
		result.upper = (this->upper * other.getLower()) - (this->lower * other.getUpper());

		bi cur_gcd = GCD(ABS(result.lower), ABS(result.upper));
		result.lower /= cur_gcd;
		result.upper /= cur_gcd;

		if (result.lower < 0) {
			result.lower *= -1;
			result.upper *= -1;
		}

		return result;
	}

	// a/b * c/d = ac/bd
	fraction operator*(const fraction& other) const {
		fraction result;
		result.lower = this->lower * other.lower;
		result.upper = this->upper * other.upper;

		bi cur_gcd = GCD(ABS(result.lower), ABS(result.upper));
		result.lower /= cur_gcd;
		result.upper /= cur_gcd;

		if (result.lower < 0) {
			result.lower *= -1;
			result.upper *= -1;
		}

		return result;
	}

	fraction operator/(const fraction& other) const {
		fraction reversed(other.getLower(), other.getUpper());
		return (*this) * reversed;
	}

	fraction& operator=(const fraction& other) {
		if (this == &other) return *this;

		this->lower = other.lower;
		this->upper = other.upper;
		return *this;
	}

	// +=
	fraction& operator+=(const fraction& other) {
		*this = *this + other;
		return *this;
	}

	// -=
	fraction& operator-=(const fraction& other) {
		*this = *this - other;
		return *this;
	}

	// *=
	fraction& operator*=(const fraction& other) {
		*this = *this * other;
		return *this;
	}

	// /=
	fraction& operator/=(const fraction& other) {
		*this = *this / other;
		return *this;
	}

	// easy output
	friend ostream& operator<<(ostream& out, const fraction& output) {
		out << output.upper << '/' << output.lower;
		return out;
	}

	~fraction() {};
};

// operator precedence
int precedence(char op) {
	if (op == '+' || op == '-') return 1;
	if (op == '*' || op == '/') return 2;
	return 0;
}

// apply operation
fraction applyOperation(const fraction& a, const fraction& b, char op) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	case '/': return a / b;
	default: throw UnknownOperatorError("Unknown operator: " + string(1, op));
	}
}

// infix to postfix
vector<string> infixToPostfix(const string& expression) {
	stack<char> operators;
	vector<string> postfix;
	stringstream ss(expression);
	string token;

	while (ss >> token) {
		if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
			postfix.push_back(token);
		}
		else if (token == "(") {
			operators.push('(');
		}
		else if (token == ")") {
			while (!operators.empty() && operators.top() != '(') {
				postfix.push_back(string(1, operators.top()));
				operators.pop();
			}
			if (!operators.empty()) operators.pop();
		}
		else {
			while (!operators.empty() && precedence(operators.top()) >= precedence(token[0])) {
				postfix.push_back(string(1, operators.top()));
				operators.pop();
			}
			operators.push(token[0]);
		}
	}
	while (!operators.empty()) {
		postfix.push_back(string(1, operators.top()));
		operators.pop();
	}
	return postfix;
}

// evaluate postfix
fraction evaluatePostfix(const vector<string>& postfix) {
	stack<fraction> st;
	for (int i = 0; i < postfix.size(); i++) {
		string token = postfix[i];

		if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
			bi upper, lower = 1;
			if (token.find('/') != string::npos) {
				std::istringstream iss(token);
				char separator;
				iss >> upper >> separator >> lower;
			}
			else {
				upper = stoll(token);
			}
			st.push(fraction(upper, lower));
		}
		else {
			if (st.size() < 2) throw InvalidExpressionError("Invalid expression: " + token);
			fraction b = st.top(); st.pop();
			fraction a = st.top(); st.pop();
			st.push(applyOperation(a, b, token[0]));
		}
	}
	if (st.size() != 1) throw InvalidExpressionError("Invalid expression");
	return st.top();
}

// help menu
void help() {
	color(10);
	cout << "Commands" << endl;
	
	color(8);
	cout << "/help : this page" << endl;
	cout << "/calc : start calculating" << endl;
	cout << "/logs : check the logs" << endl;
	cout << "/exit : exit the program" << endl;

	cout << endl;
	color(15);
	press_enter();
	cin.ignore();
	clear();

	return;
}

// calc menu
void calc() {
	cin.ignore();
	string expression;

	while (1) {

		color(12);
		cout << "WARNING! expressions without a blank does not work!" << endl;
		color(2);
		cout << "If you want to exit, type \'q\'" << endl;
		color(10);
		cout << "Input a mathematical expression: ";
		color(15);

		getline(cin, expression);

		if (expression == "q") {
			clear();
			break;
		}

		try {
			vector<string> postfix = infixToPostfix(expression);
			fraction result = evaluatePostfix(postfix);
			cout << "Result: " << result << endl;

			before.push_back(expression + " = " + result.toString());
		}
		catch (const exception& e) {
			cerr << "Error: " << e.what() << endl;
		}

		press_enter();
		clear();
	}

	return;
}

// logs menu
void logs() {
	if (before.size() == 0) {
		color(12);
		cout << "log empty!" << endl << endl;
	}
	else {
		for (int i = 0; i < before.size(); i++) {
			color(10);
			cout << '[' << i + 1 << "] ";

			color(8);
			cout << before[i] << endl;
		}
	}

	cout << endl;
	color(15);
	press_enter();
	cin.ignore();
	clear();

	return;
}

int main() {
	string input;

	while (1) {

		color(10);
		cout << "Fraction Calculator" << endl;

		color(8);
		cout << "/help for commands" << endl << endl;

		color(15);
		cin >> input;
		
		if (input == "/help") {
			clear();
			help();
		}
		else if (input == "/calc") {
			clear();
			calc();
		}
		else if (input == "/logs") {
			clear();
			logs();
		}
		else if (input == "/exit") {
			clear();
			color(15);
			break;
		}
		else {
			clear();
		}
	}

	return 0;
}