#include<iostream>
#include<vector>
#include<set>
#include<fstream>
#include <string>
#include<map>
#include<algorithm>
#include <limits.h>
#include<bits/stdc++.h>
#include<queue>

using namespace std;

#define NEG_INF INT_MIN
#define POS_INF INT_MAX

vector<string> lines;
int curIndex = 0;
int quanta = 100, initx = 50, inity = 400;
bool backedge[5000][5000];
int cyclomaticComplexity = 2;


class Node{
public:
    int lineNo;
    int x, y, positionInLevel;
    vector<Node*> children;
    vector<string> conditions;
    vector<char> operators;
    vector<string> hiddenConditions;
    vector<char> hiddenOperators;
    vector<string> opConditions;
    vector<char> opOperators;
    bool hasBackedge;
    bool loopInitiated, isLoop;
    int loopStart, loopEnd, loopIncrement;
public:
    Node(int plineNo){
        lineNo = plineNo;
        hasBackedge = false;
    }
    void addChild(Node* node);
};

class window{
public:
    int minn, maxx;
public:
    window(int mi,int ma){
        minn = mi;
        maxx = ma;
    }
};

class varRanges{
public:
    string var;
    vector<window> windows;
public:
    varRanges(string ivar){
        var = ivar;
    }
};

vector<varRanges> vars;
vector<varRanges> hiddenElse;
Node *root;

int mystoi(string s){
    //cout<< "mystoil: "<< s.length()<< endl;
    int ret = 0;
    for(int i=0;i<s.length();i++){
        if(s[i]>='0' && s[i]<='9'){
            ret *= 10;
            ret += s[i] - '0';
        }

    }
    //cout<< "mystoi: " << ret << endl;
    return ret;
}

string getConditionInsideFor(string s)
{

    int index = 0;
    string tem = "";
    while(s[index]!=';'){
        index++;
    }
    index++;
    while(s[index]!=';'){
        tem += s[index];
        index++;
    }
    return tem;
}

string extractVar(string s)
{
    string tem = "";
    int index  = 0;
    while(index<s.size() && (s[index]!= '<' &&  s[index]!='>' && s[index]!='=' && s[index]!='!')){
        if(s[index] == '|' || s[index] == '&'){
            index++;
            continue;
        }
        tem += s[index];
        index++;
    }
    return tem;
}

int extractValue(string s)
{
    string tem = "";
    int index  = s.size()-1;
    while(index>=0 && (s[index]!='<' && s[index]!='>' && s[index]!='=' && s[index]!='!')){
        tem += s[index];
        index--;
    }
    reverse(tem.begin(), tem.end());
    //cout<< "tem: " << tem<< endl;
    int ret = mystoi(tem);
    //cout<< "extractvalue: " << s << " "<< ret << endl;
    return ret;
}

char extractOP(string s){
    int index  = 0;
    while(index< s.size() && (s[index]!='<' && s[index]!='>' && s[index]!='=' && s[index]!='!')){
        index++;
    }
    return s[index];
}

bool varExist(string v, vector<varRanges> vec)
{
    for(int i=0; i<vec.size();i++){
        if(v == vec[i].var){
            return true;
        }
    }
    return false;
}

void Node:: addChild(Node *node)
{
    children.push_back(node);
}

string trim(string s)
{
    int i=0, j=0;
    while(s[i]==' '){
        i++;
    }
    s = s.substr(i, s.size());
    i = s.size();
    while(s[i]==' ' || s[i]=='\n'){
        i--;
    }
    s = s.substr(0, i);
    return s;
}

bool containsIf(string s)
{
    s = s.substr(0,2);
    return s == "if";
}

bool containsElseIf(string s)
{
    s = s.substr(0,7);
    return s == "else if";
}

bool containsElse(string s)
{
    s = s.substr(0,4);
    return s == "else";
}

bool containsWhile(string s)
{
    s = s.substr(0,5);
    return s == "while";
}

bool containsFor(string s)
{
    return s.substr(0, 3) == "for";
}

bool containsLoop(string s)
{
    return containsFor(s) || containsWhile(s);
}

bool isEnd(string s)
{
    return s[s.size()-1]=='}';
}

void readFile(string fileName)
{
    ifstream myFile(fileName.c_str());
    string temp;
    while(getline(myFile, temp)){
        lines.push_back(trim(temp));
    }

}

bool containsMain(string s)
{
    return s.substr(0,10) == "int main()";
}

string getItemsInsideIf(string s)
{
    int index = 3;
    string ret = "";
    while(s[index]!=')'){
        ret += s[index];
        index++;
    }
    return ret;
}

char alter(char op){
    if(op == '<'){
        return '>';
    }
    else if(op == '>'){
        return '<';
    }
    else if(op == '='){
        return '!';
    }
    else if(op == '!'){
        return '=';
    }
    else if(op == '&'){
        return '|';
    }
    else{
        return '&';
    }
}


string getItemsInsideElseIf(string s)
{
    int index = 8;
    string ret = "";
    while(s[index]!=')'){
        ret += s[index];
        index++;
    }
    return ret;
}

vector<string> detachIfMultipleCondition(string s)
{
    int index = 0;
    vector<string> toRet;
    string tem = "";
    while(index < s.size())
    {
        tem += s[index];
        if(index+1 < s.size() && ((s[index] == '&' && s[index+1] == '&') || (s[index] == '|' && s[index+1] == '|'))){
            tem[tem.size()-1]='\0';
            toRet.push_back(trim(tem));
            index+=2;
            tem = "";
            continue;
        }

        index++;
    }
    toRet.push_back(trim(tem));
    return toRet;
}

vector<char> getOperators(string s)
{
    int index = 0;
    vector<char> toRet;
    while(index < s.size())
    {
        if(index+1 < s.size() && ((s[index] == '&' && s[index+1] == '&') || (s[index] == '|' && s[index+1] == '|'))){
            if(s[index] == '&' && s[index+1] == '&')
                toRet.push_back('&');
            else
                toRet.push_back('|');
            index+=2;
            continue;
        }
        index++;
    }
    return toRet;
}

string getItemsInsideLoop(string s)
{
    string toRet = "";
    if(containsWhile(s)){
        int index = 6;
        while(index< s.size() && s[index]!=')'){
            toRet += s[index];
            index++;
        }
    }
    else{

    }
    return toRet;
}

Node* buildCFG(Node *root, bool isLoop)
{

    Node* parent = root;
    vector<Node*> branches;
    while(curIndex < lines.size())
    {
        Node* curNode = new Node(curIndex);

        if(containsLoop(lines[curIndex]) || containsIf(lines[curIndex])){
            //cout<< "loop" <<endl;
            cyclomaticComplexity++;
            if(branches.size()>0){
                for(int i=0;i<branches.size(); i++){
                    branches[i]->children.push_back(curNode);
                }
                branches.clear(); //test
            }
            else{
                parent->children.push_back(curNode);
            }
            curIndex++;
            if(containsLoop(lines[curIndex-1]))
            {
                branches.push_back(curNode);
                if(containsWhile(lines[curIndex-1])){
                    string tem = getItemsInsideLoop(lines[curIndex-1]);
                    //cout << "tem: " << tem << endl;
                    vector<string> conditions = detachIfMultipleCondition(tem);
                    vector<char> operators = getOperators(tem);
                    curNode->operators = operators;
                    curNode->conditions = conditions;
                }
                else if(containsFor(lines[curIndex-1])){
                    string tem = getConditionInsideFor(lines[curIndex-1]);
                    vector<string> conditions = detachIfMultipleCondition(tem);
                    vector<char> operators = getOperators(tem);
                    curNode->operators = operators;
                    curNode->conditions = conditions;
                    cyclomaticComplexity++;
                }
                buildCFG(curNode, true);
            }
            else
            {
                string tem = getItemsInsideIf(lines[curIndex-1]);
                //cout << "tem: " << tem << endl;
                vector<string> conditions = detachIfMultipleCondition(tem);
                vector<char> operators = getOperators(tem);
                curNode->operators = operators;
                curNode->conditions = conditions;
                Node* temp = buildCFG(curNode, false);
                branches.push_back(temp);
            }
        }
        else if(containsElseIf(lines[curIndex]) || containsElse(lines[curIndex])){
            if(containsElseIf(lines[curIndex])){
                string tem = getItemsInsideElseIf(lines[curIndex]);
                vector<string> conditions = detachIfMultipleCondition(tem);
                vector<char> operators = getOperators(tem);
                curNode->operators = operators;
                curNode->conditions = conditions;
                cyclomaticComplexity++;
            }

            parent->children.push_back(curNode);
            curIndex++;
            branches.push_back(buildCFG(curNode, false));
        }
        else{
            if(branches.size()>0){
                for(int i=0;i<branches.size();i++){
                    branches[i]->children.push_back(curNode);
                }
                branches.clear();

            }
            else{
                parent->addChild(curNode);
            }
            curIndex++;
            if(isEnd(lines[curIndex-1])){
                //cout << "end" <<endl;
                if(isLoop){
                    root->hasBackedge = true;
                    backedge[curNode->lineNo][root->lineNo] =  true;
                    curNode->children.push_back(root);
                }
                return curNode;
            }
            parent = curNode;
        }
    }
    return NULL;
}

bool visited[5000] = {false};
int graph[5000][5000] = {0};
int levelNodeCount[5000];


void assignCoordinates(Node* current, int level)
{
    visited[current->lineNo] = true;
    if(levelNodeCount[level]%2 == 1)
    {
        current->y = inity + (current->positionInLevel - (levelNodeCount[level]+1)/2) * quanta;
    }
    else{
        current->y = inity - 75 + (current->positionInLevel - (levelNodeCount[level]+1)/2) * quanta;
    }
    //cout<< current->x << " "<< current->y <<endl;
    for(int i=0;i< current->children.size(); i++){
        if(!visited[current->children[i]->lineNo]){
            assignCoordinates(current->children[i], level+1);
        }
    }
}



void traverse(Node* current, int level)
{
    //cout<< "node: " << current->lineNo+1 << " level: " << level << endl;
    levelNodeCount[level]++;
    current->x = initx + quanta*level;
    current->positionInLevel = levelNodeCount[level];
    visited[current->lineNo] = true;

    for(int i=0;i< current->children.size(); i++){
        if(!visited[current->children[i]->lineNo]){
            //cout<< "child: " << current->children[i]->lineNo+1 << endl;
            traverse(current->children[i], level+1);
        }
    }

}

void reset()
{
    for(int i=0;i<5000;i++){
        visited[i] = false;
    }
}


window* getWindow(char op, int value, string var){
    int minn, maxx;
    if(op == '<'){
        minn = NEG_INF;
        maxx = value-1;
    }else if(op == '>'){
        minn = value+1;
        maxx = POS_INF;
    }
    else if(op == '='){
        minn = value;
        maxx = value;
    }
    else if(op == '!'){
        //cout<< "called" <<endl;
        bool ff=false;
        if(varExist(var, vars)){
            for(int i=0; i<vars.size();i++){
                if(var == vars[i].var){
                    for(int j=0; j< vars[i].windows.size(); j++){
                        //cout<< vars[i].windows[j].minn << " " << vars[i].windows[j].maxx <<endl;
                        if(vars[i].windows[j].minn == NEG_INF){
                            //cout<< "hello: "<< var << " " << value << endl;
                            minn = NEG_INF;
                            maxx = value-1;
                            ff=true;
                        }
                    }
                }
                if(!ff){
                    //cout<< "siam" <<endl;
                    maxx = POS_INF;
                    minn = value+1;
                }
            }
        }
        else{
            minn = NEG_INF;
            maxx = value-1;
        }

    }
    window* toRet = new window(minn, maxx);
    return toRet;
}

void prepareVariableWindows(Node* current, int level, bool hide)
{
    //cout<< "c" <<endl;
    visited[current->lineNo] = true;
    vector<string> conditions = current->conditions;
    vector<char> operators = current->operators;
    int opCount = operators.size();
    int i,j,leftCount=0, rightCount=0;
    if(opCount==0) {
        leftCount = 1;
        rightCount = 2;
    }
    for(i=0;i<opCount; i++){
        if(operators[i] == '|') break;
        leftCount++;
    }
    for(j=opCount-1; j>=0;j--){
        if(operators[j] == '|') break;
        rightCount++;
    }
    if(leftCount <= rightCount){
        for(int i=0; i< leftCount+1 && i<conditions.size(); i++){

            string var = extractVar(conditions[i]);
            int value = extractValue(conditions[i]);
            char op = extractOP(conditions[i]);
            window *wn = getWindow(op, value, var);
            if(varExist(var, vars)){
                for(int k=0; k<vars.size(); k++){
                    if(var == vars[k].var){
                        vars[k].windows.push_back(*wn);
                    }
                }
            }else{
                varRanges *varR = new varRanges(var);
                varR->windows.push_back(*wn);
                vars.push_back(*varR);
            }
        }
    }
    else{
        for(int j=opCount, i=0; i< rightCount + 1 ; i++, j--){
            string var = extractVar(conditions[j]);
            int value = extractValue(conditions[j]);
            char op = extractOP(conditions[j]);
            window *wn = getWindow(op, value, var);
            if(varExist(var, vars)){
                for(int k=0; k<vars.size(); k++){
                    if(var == vars[k].var){
                        vars[k].windows.push_back(*wn);
                    }
                }
            }else{
                varRanges *varR = new varRanges(var);
                varR->windows.push_back(*wn);
                vars.push_back(*varR);
            }

        }
    }

    vector<string> opConditions = current->opConditions;
    vector<char> opOperators = current->opOperators;
    opCount = opOperators.size();
    leftCount=0, rightCount=0;
    if(opCount==0) {
        leftCount = 1;
        rightCount = 2;
    }
    for(i=0;i<opCount; i++){
        if(opOperators[i] == '|') break;
        leftCount++;
    }
    for(j=opCount-1; j>=0;j--){
        if(opOperators[j] == '|') break;
        rightCount++;
    }
    if(leftCount <= rightCount){
        for(int i=0; i< leftCount+1 && i<opConditions.size(); i++){
            string var = extractVar(opConditions[i]);
            int value = extractValue(opConditions[i]);
            char op = extractOP(opConditions[i]);
            window *wn = getWindow(op, value, var);
            if(varExist(var, vars)){
                for(int k=0; k<vars.size(); k++){
                    if(var == vars[k].var){
                        vars[k].windows.push_back(*wn);
                    }
                }
            }else{
                varRanges *varR = new varRanges(var);
                varR->windows.push_back(*wn);
                vars.push_back(*varR);
            }
        }
    }
    else{
        for(int j=opCount, i=0; i< rightCount + 1 ; i++, j--){
            string var = extractVar(opConditions[j]);
            int value = extractValue(opConditions[j]);
            char op = extractOP(conditions[j]);
            window *wn = getWindow(op, value, var);
            if(varExist(var, vars)){
                for(int k=0; k<vars.size(); k++){
                    if(var == vars[k].var){
                        vars[k].windows.push_back(*wn);
                    }
                }
            }else{
                varRanges *varR = new varRanges(var);
                varR->windows.push_back(*wn);
                vars.push_back(*varR);
            }
        }
    }

    if(hide){
        vector<string> opConditions = current->hiddenConditions;
        vector<char> opOperators = current->hiddenOperators;
        opCount = opOperators.size();
        leftCount=0, rightCount=0;
        if(opCount==0) {
            leftCount = 1;
            rightCount = 2;
        }
        for(i=0;i<opCount; i++){
            if(opOperators[i] == '|') break;
            leftCount++;
        }
        for(j=opCount-1; j>=0;j--){
            if(opOperators[j] == '|') break;
            rightCount++;
        }
        if(leftCount <= rightCount){
            for(int i=0; i< leftCount+1 && i<opConditions.size(); i++){
                string var = extractVar(opConditions[i]);
                int value = extractValue(opConditions[i]);
                char op = extractOP(opConditions[i]);
                window *wn = getWindow(op, value, var);
                if(varExist(var, vars)){
                    for(int k=0; k<vars.size(); k++){
                        if(var == vars[k].var){
                            vars[k].windows.push_back(*wn);
                        }
                    }
                }else{
                    varRanges *varR = new varRanges(var);
                    varR->windows.push_back(*wn);
                    vars.push_back(*varR);
                }
            }
        }
        else{
            for(int j=opCount, i=0; i< rightCount + 1 ; i++, j--){
                string var = extractVar(opConditions[j]);
                int value = extractValue(opConditions[j]);
                char op = extractOP(conditions[j]);
                window *wn = getWindow(op, value, var);
                if(varExist(var, vars)){
                    for(int k=0; k<vars.size(); k++){
                        if(var == vars[k].var){
                            vars[k].windows.push_back(*wn);
                        }
                    }
                }else{
                    varRanges *varR = new varRanges(var);
                    varR->windows.push_back(*wn);
                    vars.push_back(*varR);
                }
            }
        }
    }

    for(int i=0;i< current->children.size(); i++){
        if(!visited[current->children[i]->lineNo]){
            prepareVariableWindows(current->children[i], level+1, hide);
        }
    }
}

bool compareInterval(window w1, window w2)
{
    if(w1.minn == w2.minn){
        return w1.maxx < w2.maxx;
    }
    return w1.minn < w2.minn ;
}

void compressWindow()
{
    for(int i = 0; i< vars.size(); i++){
        sort(vars[i].windows.begin(), vars[i].windows.end(), compareInterval);
        for(int j=0; j< vars[i].windows.size(); j++){
            for(int k=j+1; k< vars[i].windows.size(); k++){
                if(j==k) continue;
                if(vars[i].windows[j].minn <= vars[i].windows[k].minn && vars[i].windows[j].maxx>= vars[i].windows[k].maxx){
                    vars[i].windows.erase(vars[i].windows.begin()+j);
                }
                else if(vars[i].windows[j].maxx >= vars[i].windows[k].minn){
                    vars[i].windows[j].minn = vars[i].windows[k].minn;
                    vars[i].windows[j].maxx = min(vars[i].windows[j].maxx, vars[i].windows[k].maxx);
                    vars[i].windows.erase(vars[i].windows.begin()+k);
                }

            }
        }
    }

}

void findElseCondition(Node* root)
{
    queue<Node*> q;
    q.push(root);
    string last, elseStatement;
    visited[root->lineNo] = true;
    while(!q.empty()){
        Node* cur = q.front();
        q.pop();

        visited[cur->lineNo] = true;
        if(cur->children.size()>1){
            bool elsePresent = false, ifPresent = false;
            vector<string> conditions;
            vector<char> operators;
            Node* prev;
            for(int i=0; i<cur->children.size();i++){
                if(containsIf(lines[cur->children[i]->lineNo])){
                    last =  lines[cur->children[i]->lineNo];
                    conditions = cur->children[i]->conditions;
                    operators = cur->children[i]->operators;
                    ifPresent = true;
                }
                else if(containsElseIf(lines[cur->children[i]->lineNo])){
                    string tem;
                    if(containsIf(lines[prev->lineNo])){
                        tem = getItemsInsideIf(last);
                    }
                    else if(containsElseIf(lines[prev->lineNo])){
                        tem = getItemsInsideElseIf(last);
                    }
                    vector<string> conditions = detachIfMultipleCondition(tem);

                    vector<char> operators = getOperators(tem);
                    for(int j=0;j< operators.size();j++){
                        operators[j] = alter(operators[j]);
                    }
                    for(int k=0;k<conditions.size();k++){
                        for(int j=0;j<conditions[k].length();j++){
                            if(conditions[k][j] == '>' || conditions[k][j] == '<' || conditions[k][j] == '=' || conditions[k][j] == '!' || conditions[k][j] == '&' || conditions[k][j] == '|'){
                                conditions[k][j] = alter(conditions[k][j]);
                            }
                        }
                    }
                    for(int j=0;j<conditions.size();j++){
                        cur->children[i]->opConditions.push_back(conditions[j]);
                    }
                    for(int j=0;j<operators.size();j++){
                        cur->children[i]->opOperators.push_back(operators[j]);
                    }
                    last =  lines[cur->children[i]->lineNo];
                }
                else if(containsElse(lines[cur->children[i]->lineNo])){
                    string tem = getItemsInsideElseIf(last);
                    vector<string> conditions = detachIfMultipleCondition(tem);
                    vector<char> operators = getOperators(tem);
                    for(int j=0;j< operators.size();j++){
                        operators[j] = alter(operators[j]);
                    }
                    for(int k=0;k<conditions.size();k++){
                        for(int j=0;j<conditions[k].length();j++){
                            if(conditions[k][j] == '>' || conditions[k][j] == '<' || conditions[k][j] == '=' || conditions[k][j] == '!' || conditions[k][j] == '&' || conditions[k][j] == '|'){
                                conditions[k][j] = alter(conditions[k][j]);
                            }
                        }
                    }

                    cur->children[i]->operators = operators;
                    cur->children[i]->conditions = conditions;
                    elsePresent = true;
                }
                prev = cur->children[i];
            }
            if(ifPresent&&!elsePresent){
                string tem = getItemsInsideElseIf(last);
                vector<string> conditions = detachIfMultipleCondition(tem);
                vector<char> operators = getOperators(tem);
                for(int j=0;j< operators.size();j++){
                    operators[j] = alter(operators[j]);
                }
                for(int k=0;k<conditions.size();k++){
                    for(int j=0;j<conditions[k].length();j++){
                        if(conditions[k][j] == '>' || conditions[k][j] == '<' || conditions[k][j] == '=' || conditions[k][j] == '!' || conditions[k][j] == '&' || conditions[k][j] == '|'){
                            conditions[k][j] = alter(conditions[k][j]);
                        }
                    }
                }
                cur->children[cur->children.size()-1]->hiddenOperators = operators;
                cur->children[cur->children.size()-1]->hiddenConditions = conditions;
            }
        }
        for(int i=0; i<cur->children.size();i++){
            if(!visited[cur->children[i]->lineNo])
                q.push(cur->children[i]);
        }
    }
}

void initStatementCoverage()
{
    root = new Node(curIndex);
    curIndex++;
    buildCFG(root, false);
    cout<< "cyclomatic complexity: " << cyclomaticComplexity << endl;
    reset();
    findElseCondition(root);
    reset();
    traverse(root, 0);
    reset();
}

int getValueWithinWindow(window w)
{
    if(w.maxx ==  POS_INF){
        return w.minn;
    }else if(w.minn == NEG_INF){
        return w.maxx;
    }else{
        return (w.minn+w.maxx)/2;
    }
}



void generateTestCases()
{
    int tc = 0;
    for(int i=0;i< vars.size(); i++){
        for(int j=i+1;j<vars.size();j++){
            if(i==j) continue;
            for(int k=0;k< vars[i].windows.size();k++){
                for(int l=0; l< vars[j].windows.size();l++){
                    tc++;
                    cout<< "Test case: " << tc << endl;
                    cout<< vars[i].var << " = "<< getValueWithinWindow(vars[i].windows[k]) << " ";
                    cout<< vars[j].var << " = "<< getValueWithinWindow(vars[j].windows[l]) << endl;
                }
            }
        }
    }
}


void conditionHelper(Node *cur)
{
    visited[cur->lineNo] = true;
    vector<string> conditions = cur->conditions;
    vector<char> operators = cur->operators;

    for(int i=0; i<conditions.size(); i++){

        string var = extractVar(conditions[i]);
        int value = extractValue(conditions[i]);
        char op = extractOP(conditions[i]);
        window *wn = getWindow(op, value, var);
        if(varExist(var, vars)){
            for(int k=0; k<vars.size(); k++){
                if(var == vars[k].var){
                    vars[k].windows.push_back(*wn);
                }
            }
        }else{
            varRanges *varR = new varRanges(var);
            varR->windows.push_back(*wn);
            vars.push_back(*varR);
        }
    }

    for(int j=0;j< operators.size();j++){
        operators[j] = alter(operators[j]);
    }
    for(int k=0;k<conditions.size();k++){
        for(int j=0;j<conditions[k].length();j++){
            if(conditions[k][j] == '>' || conditions[k][j] == '<' || conditions[k][j] == '=' || conditions[k][j] == '!' || conditions[k][j] == '&' || conditions[k][j] == '|'){
                conditions[k][j] = alter(conditions[k][j]);
            }
        }
    }

    for(int i=0; i<conditions.size(); i++){

        string var = extractVar(conditions[i]);
        int value = extractValue(conditions[i]);
        char op = extractOP(conditions[i]);
        window *wn = getWindow(op, value, var);
        if(varExist(var, vars)){
            for(int k=0; k<vars.size(); k++){
                if(var == vars[k].var){
                    vars[k].windows.push_back(*wn);
                }
            }
        }else{
            varRanges *varR = new varRanges(var);
            varR->windows.push_back(*wn);
            vars.push_back(*varR);
        }
    }

    for(int i=0; i<cur->children.size();i++){
        if(!visited[cur->children[i]->lineNo])
            conditionHelper(cur->children[i]);
    }


}

void conditionCoverage(Node* current, int level)
{
    root = new Node(curIndex);
    curIndex++;
    buildCFG(root, false);
    reset();
    conditionHelper(root);
}


void menu()
{
    while(1)
    {
        lines.clear();
        curIndex = 0;
        cyclomaticComplexity = 2;
        vars.clear();
        cout<< "\nOption :\n\n";
        cout << "1. Test case for Statement Coverage\n2. Test Case for Branch Coverage\n3. Test Case for Condition Coverage\n";
        cout<< "\n\n";
        int choice;
        cout<< "\nChoose one option : \n";
        cin>> choice;

        cout << "Enter file name: ";
        string filename;
        cin >> filename;
        readFile(filename);
        if(choice == 1){
            initStatementCoverage();
            prepareVariableWindows(root, 0, false);

            compressWindow();

            reset();
            generateTestCases();
        }
        else if(choice == 2){
            initStatementCoverage();
            prepareVariableWindows(root, 0, true);
            compressWindow();
            reset();
            generateTestCases();
        }
        else if(choice == 3){
            conditionCoverage(root, 0);
            compressWindow();
            reset();
            generateTestCases();
        }

    }
}


int main()
{
    menu();

}


