#include <string>
#include <vector>
#include <cstdio>

using namespace std;

static void
split_fuzzy(const string& src, char delim, vector<string>& elementVec)
{
    printf("split_fuzzy: src=\"%s\", delim='%c'\n", src.c_str(), delim);

    string delimiter(" ");
    delimiter += delim;

    string::size_type startPos(0);
    string str(src);

    printf("find_first_of\n");
    string::size_type endPos = str.find_first_of(delimiter);

    while (endPos != string::npos)
    {
        printf("iter: startPos=%zu, endPos=%zu\n", startPos, endPos);

        elementVec.push_back(string(str, startPos, endPos - startPos));

        printf("1\n");
        string::size_type nextPos = str.find_first_not_of(delimiter, endPos);
        printf("2: nextPos=%zu (npos=%zu)\n", nextPos, string::npos);

        if (nextPos == string::npos) {
            str.erase(startPos);
            break;
        }

        str = str.erase(startPos, nextPos - startPos);
        printf("2.5: str after erase=\"%s\"\n", str.c_str());

        endPos = str.find_first_of(delimiter);
        printf("3: endPos=%zu\n", endPos);
    }

    printf("end: push_back(\"%s\")\n", str.c_str());
    elementVec.push_back(str);
}

static void
test(const string& src, char delim)
{
    printf("\n=== TEST: src=\"%s\", delim='%c' ===\n", src.c_str(), delim);
    vector<string> vec;
    split_fuzzy(src, delim, vec);
    printf("Result (%zu elements):\n", vec.size());
    for (size_t i = 0; i < vec.size(); i++) {
        printf("  [%zu] = \"%s\"\n", i, vec[i].c_str());
    }
}

int main()
{
    test("a,b,c", ',');
    test("a, b, c", ',');
    test("a b c", ',');
    test("a,,b", ',');
    test("a, ,b", ',');

    test("", ',');
    test(",", ',');
    test(",,,", ',');
    test("abc", ',');
    test("  a  b  ", ' ');

    test("a, b c,d", ',');
    test("a, b c,d", ' ');

    string longstr;
    for (int i = 0; i < 100; i++) {
        longstr += "word";
        if (i % 3 == 0) longstr += ",";
        else if (i % 3 == 1) longstr += " ";
    }
    test(longstr, ',');

    printf("\nAll tests passed!\n");
    return 0;
}