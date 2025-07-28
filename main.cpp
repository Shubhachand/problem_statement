#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <limits>
#include <set>

using namespace std;

typedef long long ll;

ll baseToDecimal(const string& val, int base) {
    ll res = 0;
    for (char ch : val) {
        int digit;
        if (isdigit(ch)) digit = ch - '0';
        else digit = tolower(ch) - 'a' + 10;
        res = res * base + digit;
    }
    return res;
}

long double lagrangeInterpolation(const vector<pair<int, ll>>& points) {
    int k = points.size();
    long double result = 0.0;

    for (int i = 0; i < k; ++i) {
        long double xi = points[i].first;
        long double yi = points[i].second;
        long double term = yi;

        for (int j = 0; j < k; ++j) {
            if (i == j) continue;
            long double xj = points[j].first;
            if (xi == xj) return numeric_limits<ll>::min(); // skip invalid
            term *= (-xj) / (xi - xj);
        }
        result += term;
    }
    return llround(result);
}

void generateCombinations(const vector<pair<int, ll>>& shares, int k, int start,
                          vector<pair<int, ll>>& current,
                          vector<vector<pair<int, ll>>>& allComb) {
    if (current.size() == k) {
        allComb.push_back(current);
        return;
    }
    for (int i = start; i < shares.size(); ++i) {
        current.push_back(shares[i]);
        generateCombinations(shares, k, i + 1, current, allComb);
        current.pop_back();
    }
}

string clean(const string& s) {
    string out;
    for (char c : s) {
        if (!isspace(c)) out += c;
    }
    return out;
}

int extractNumberAfter(const string& str, const string& key) {
    size_t pos = str.find(key);
    if (pos == string::npos) return -1;
    pos += key.length();
    string number = "";
    while (pos < str.size() && isdigit(str[pos])) {
        number += str[pos++];
    }
    return stoi(number);
}

ll processFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return -1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string rawJson = clean(buffer.str());

    int n = extractNumberAfter(rawJson, "\"n\":");
    int k = extractNumberAfter(rawJson, "\"k\":");

    map<int, pair<int, string>> shareMap;
    for (size_t i = 1; i < rawJson.length(); ++i) {
        if (isdigit(rawJson[i]) && rawJson[i - 1] == '"') {
            int x = 0;
            while (i < rawJson.length() && isdigit(rawJson[i])) {
                x = x * 10 + (rawJson[i++] - '0');
            }

            string baseStr, valStr;
            size_t basePos = rawJson.find("\"base\":\"", i);
            if (basePos != string::npos) {
                basePos += 8;
                while (basePos < rawJson.length() && rawJson[basePos] != '"') baseStr += rawJson[basePos++];
            }

            size_t valPos = rawJson.find("\"value\":\"", basePos);
            if (valPos != string::npos) {
                valPos += 9;
                while (valPos < rawJson.length() && rawJson[valPos] != '"') valStr += rawJson[valPos++];
            }

            if (!baseStr.empty() && !valStr.empty()) {
                shareMap[x] = {stoi(baseStr), valStr};
            }
        }
    }

    vector<pair<int, ll>> shares;
    for (auto& entry : shareMap) {
        int x = entry.first;
        int base = entry.second.first;
        string val = entry.second.second;
        ll y = baseToDecimal(val, base);
        shares.push_back({x, y});
    }

    vector<vector<pair<int, ll>>> allComb;
    vector<pair<int, ll>> current;
    generateCombinations(shares, k, 0, current, allComb);

    if (allComb.empty()) {
        cerr << "No combinations generated.\n";
        return -1;
    }

    map<ll, int> secretCount;
    for (auto& comb : allComb) {
        set<int> x_seen;
        bool duplicate_x = false;
        for (auto& p : comb) {
            if (!x_seen.insert(p.first).second) {
                duplicate_x = true;
                break;
            }
        }
        if (duplicate_x) continue;

        ll secret = (ll)lagrangeInterpolation(comb);
        if (secret != numeric_limits<ll>::min() && secret >= 0 && secret < (1LL << 60)) {

            secretCount[secret]++;
        }
    }

    if (secretCount.empty()) {
        cerr << "Interpolation failed for all combinations.\n";
        return -1;
    }

    ll finalSecret = -1;
    int maxFreq = 0;
    for (auto it = secretCount.begin(); it != secretCount.end(); ++it) {
        if (it->second > maxFreq) {
            maxFreq = it->second;
            finalSecret = it->first;
        }
    }
    return finalSecret;
}

int main() {
    ll s1 = processFile("testcase1.json");
    ll s2 = processFile("testcase2.json");
    cout << "Secret 1: " << s1 << endl;
    cout << "Secret 2: " << s2 << endl;
    return 0;
}
