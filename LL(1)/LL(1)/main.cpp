//
//  main.cpp
//  LL(1)
//
//  Created by Dousir9 on 2020/12/7.
//  Copyright © 2020 🍓XJK. All rights reserved.
//

#include <iostream>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
using namespace std;

map<char, set<string>> p; //产生式
map<char, set<pair<string, set<char>>>> select; //select集
map<char, int> nul; //是否可以推导出空
map<char, set<char>> first, follow, includ; //first集、follow集
map<char, int> done; //求first集是标记是否已经处理完
set<char> vn, vt; //非终结符和终结符
map<char, map<char, string>> M; //预测分析表
char start; //开始符
const char N = '@';
const string blank = "                                                     ";

inline bool in_vt(char c) {
    if (vt.find(c) != vt.end()) return true;
    return false;
}

inline bool in_vn(char c) {
    if (vn.find(c) != vn.end()) return true;
    return false;
}

inline bool is_low(char c) {
    if (!(c >= 'A' && c <= 'Z')) return true;
    return false;
}

bool check_has_vt(const string &now) {
    int len = (int) now.size();
    for (int i = 0; i < len; i++)
        if (in_vt(now[i])) return true;
    return false;
}

int all_null(const string &now) {
    if (now == "null") return true;
    int len = (int) now.size();
    int f = 0;
    for (int i = 0; i < len; i++)
        if (nul[now[i]] == 0) {
            return 0;
        } else if (nul[now[i]] == -1) {
            f = 1;
        }
    if (f) return -1;
    return 1;
}

void init() {
    ifstream OutFile;
    OutFile.open("/Users/xumingfei/Desktop/项目/编译原理/LL(1)/production.txt");
    char c;
    string str;
    int f = 0;
    while (!OutFile.eof()) {
        OutFile >> c >> str;
        if (!f) {f = 1; start = c;}
        p[c].insert(str);
        vn.insert(c);
    }
}

void get_nul() {
    map<char, set<string>> cur(p); //复制一份产生式(因为涉及到删除，所以为了保护原产生式，在复制的这份上进行删除操作)
    set<char> now; //存放状态未定的非终结符
    for (auto it = cur.begin(); it != cur.end(); ++it) {
        nul[it->first] = -1; //状态置为待定
        for (auto i = it->second.begin(); i != it->second.end();) {
            string t = *i;
            if (t != "null") for (int j = 0, len = (int) t.size(); j < len; j++)
                if (is_low(t[j])) vt.insert(t[j]);
            if (*i == "null") {
                /*
                 该非终结符能推导出空串，将该非终结符的标志置为1，并删除该非终结符的所有产生式
                 此处不进行删除操作，而是用now存放那些状态未定的非终结符，后续中只去操作now中的元素，的这样该非终结符也就变相被”删除“了
                */
                nul[it->first] = 1;
                break;
            } else if (check_has_vt(*i)) {
                //若产生式中存在终结符，则删除该产生式
                auto d = i;
                ++i; //指针先后移，再删除
                it->second.erase(*d);
                continue;
            } else {
                //若全为非终结符，则保留该产生式
                ++i;
            }
        }
        if ((int) it->second.size() > 0 && nul[it->first] != 1) {
            // 如果该非终结符还存在右部为全为非终结符的产生式并且状态不为1，则加入now中进行后续处理
            now.insert(it->first);
        } else if (nul[it->first] == -1) {
            // 如果该非终结符不存在右部为全为非终结符的产生式并且状态为待定，则它无法推导为空串，置为0
            nul[it->first] = 0;
        }
    }
    string bk = "                                               ";
    cout << "\n\n" << bk << "一次扫描后剩下的产生式:" << '\n' << '\n';
    for (auto it = cur.begin(); it != cur.end(); ++it) {
        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
            cout << blank << it->first << " -> "<< *i << '\n';
        }
    }
    bk = "                                                 ";
    cout << "\n\n" << bk << "一次扫描后状态:" << '\n' << '\n';
    for (auto it = nul.begin(); it != nul.end(); ++it) {
        cout << blank << it->first << ": " << it->second << '\n';
    }
    cout << '\n';
    int flag; //用于记录特征有无变化
    do {
        flag = 0; //刚开始时置为无变化
        for (auto it = now.begin(); it != now.end();) {
            int f = 0; //用于判断now中是否应该删除该非终结符
            for (auto i = cur[*it].begin(); i != cur[*it].end();) {
                int check = all_null(*i);
                if (check == 1) {
                    // 若产生式右部的非终结符全都能推导出空串，则该非终结符能推导出空
                    nul[*it] = 1;
                    flag = f = 1; // (1)有变化 (2)在now中删除该非终结符(删除该非终结的所有产生式)
                    break;
                } else if (check == 0) {
                    // 若产生式右部的非终结符有的不能推导为空，则删除该产生式
                    flag = f = 1; // 在now中删除该非终结符(删除该非终结的所有产生式)
                    auto d = i;
                    ++i;
                    cur[*it].erase(d);
                } else {
                    // 若产生式右部的非终结符还处于待定状态，则暂时不管
                    ++i;
                }
            }
            // 如果没有产生式了并且该非终结的状态还是待定，则将该非终结的状态置为0并且从now中删除
            if ((int) cur[*it].size() == 0 && nul[*it] == -1) {
                nul[*it] = 0;
                f = 1;
            }
            if (f) {
                // 该非终结符能推导出空或者不能推导出空，则now中删除该非终结符
                auto d = it;
                ++it;
                now.erase(*d);
            } else {
                ++it;
            }
        }
    } while (flag);
    bk = "                                                    ";
    cout << "\n" << bk << "最终状态:" << '\n' << '\n';
    for (auto it = nul.begin(); it != nul.end(); ++it) {
        cout << blank << it->first << ": " << it->second << '\n';
    }
}

set<char> dfs(char now) {
    if (in_vt(now)) { //如果是非终结符，则first集为本身
        set<char> t;
        t.insert(now);
        return t;
    } else {
        if (done[now]) //如果该非终结符的first集已经处理过了，则直接返回即可，不必重复求解
            return first[now];
        for (auto it = p[now].begin(); it != p[now].end(); ++it) {
            string r = *it;
            if (r == "null") { //如果能推导出空串，则将空串加入first集
                first[now].insert(N);
            } else {
                int len = (int) r.size();
                for (int i = 0; i < len; i++) {
                    if (in_vn(r[i])) { //若为非终结符，进行继续进行搜索，利用搜索结果更新first集
                        auto h = dfs(r[i]); //dfs搜索该非终结符的first集
                        if (nul[r[i]] && (h.find(N) != h.end())) h.erase(N); //如果有空串，先剔除空串
                        for (auto j = h.begin(); j != h.end(); ++j) first[now].insert(*j); //补充first集
                        if (nul[r[i]] != 1) { //如果当前字母不能推导为空串，则跳出，即没有空串加入
                            break;
                        } else if (i == len - 1) { //如果当前字母能推导为空串，且是最后一个，则在fisrt集中加上空串
                            first[now].insert(N);
                        }
                    } else { //若为终结符，则first集就是它自身
                        first[now].insert(r[i]);
                        break;
                    }
                }
            }
        }
        done[now] = 1; // 打上标记，表示该非终结符的first集已经处理过了，下次使用时直接返回即可，不必重复求解
        return first[now];
    }
}

void get_first() {
    for (auto it = vn.begin(); it != vn.end(); ++it) dfs(*it); //对每个非终结符进行dfs
    string bk = "                                                    ";
    cout << "\n\n" << bk << "First集为:" << '\n' << '\n';
    for (auto it = first.begin(); it != first.end(); ++it) {
        cout << blank << "FIRST(" << it->first << ") = {";
        for (auto j = it->second.begin(); j != it->second.end();) {
            cout << *j++;
            if (j != it->second.end()) cout << ",";
        }
        cout << "} \n";
        
    }
}

void get_follow() {
    follow[start].insert('#'); //把’#‘加入文法开始符号的Follow集中
    for (auto it = p.begin(); it != p.end(); ++it) {
        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
            string r = *i;
            int len = (int) r.size();
            for (int i = 0; i < len; i++) {
                if (in_vn(r[i]) && i != len - 1) {
                    if (in_vn(r[i + 1])) { //右边为非终结符
                        //follow集中加入右边为非终结符first中的非空元素
                        for (auto it = first[r[i + 1]].begin(); it != first[r[i + 1]].end(); ++it)
                            if (*it != N) follow[r[i]].insert(*it);
                        //如果右边的非终结符能推导出空且为产生式最后一个元素，则加上产生式左边的非终结符的follow集
                        if (nul[r[i + 1]] && i + 1 == len - 1) includ[r[i]].insert(it->first);
                    } else {
                        follow[r[i]].insert(r[i + 1]); //右边的为终结符，直接将它加到follow集中
                    }
                } else if (in_vn(r[i])) {
                    includ[r[i]].insert(it->first);
                }
            }
        }
    }
    string bk = "                                                    ";
    cout << "\n\n" << bk << "includ为:" << '\n' << '\n';
    for (auto it = includ.begin(); it != includ.end(); ++it) {
        cout << blank << it->first << ":{ ";
        for (auto j = it->second.begin(); j != it->second.end(); ++j) {
            cout << *j << " ";
        }
        cout << "} \n";
    }
    int flag;
    do {
        flag = 0;
        for (auto it = includ.begin(); it != includ.end(); ++it) {
            int cur = (int) follow[it->first].size();
            for (auto i = it->second.begin(); i != it->second.end(); ++i) {
                for (auto j = follow[*i].begin(); j != follow[*i].end(); ++j) {
                    follow[it->first].insert(*j);
                }
            }
            if ((int) follow[it->first].size() > cur) flag = 1;
        }
    } while (flag);
    cout << "\n\n" << bk << "Follow集为:" << '\n' << '\n';
    for (auto it = follow.begin(); it != follow.end(); ++it) {
        cout << blank << "FOLLOW(" << it->first << ") = {";
        for (auto j = it->second.begin(); j != it->second.end();) {
            cout << *j++;
            if (j != it->second.end()) cout << ",";
        }
        cout << "} \n";
    }
}

void get_select() {
    for (auto it = p.begin(); it != p.end(); ++it) {
        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
            set<char> t;
            string r = *i;
            if (r == "null") {
                for (auto l = follow[it->first].begin(); l != follow[it->first].end(); ++l)
                    t.insert(*l);
                select[it->first].insert({r, t});
                continue;
            }
            int len = (int) r.size();
            for (int j = 0; j < len; j++) {
                if (in_vn(r[j])) {
                    for (auto k = first[r[j]].begin(); k != first[r[j]].end(); ++k)
                        if (*k != N) t.insert(*k);
                    if (nul[r[j]]) {
                        if (j == len - 1)
                            for (auto l = follow[it->first].begin(); l != follow[it->first].end(); ++l)
                                t.insert(*l);
                    } else {
                        break;
                    }
                } else {
                    t.insert(r[j]);
                    break;
                }
            }
            select[it->first].insert({r, t});
        }
    }
    string bk = "                                                    ";
    cout << "\n\n" << bk << "Select集为:" << '\n' << '\n';
    for (auto it = select.begin(); it != select.end(); ++it) {
        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
            cout << blank << "SELECT(" << it->first << "->" << i->first << ") = {";
            for (auto j = i->second.begin(); j != i->second.end();) {
                cout << *j++;
                if (j != i->second.end()) cout << ",";
            }
            cout << "}\n";
        }
    }
}

void get_M() {
    for (auto it = select.begin(); it != select.end(); ++it)
        for (auto i = it->second.begin(); i != it->second.end(); ++i)
            for (auto j = i->second.begin(); j != i->second.end(); ++j)
                M[it->first][*j] = i->first;
    string bk = "                                                    ", bkk = "       ";
    cout << "\n\n" << bk << "预测分析表为:" << '\n' << '\n';
    cout << "         " << "\t\t";
    for (auto it = vt.begin(); it != vt.end(); ++it)
        cout << *it << "\t\t";
    cout << "#" << '\n';
    for (auto it = vn.begin(); it != vn.end(); ++it) {
        cout << "         " << *it << "\t\t";
        for (auto i = vt.begin(); i != vt.end(); ++i)
            cout << M[*it][*i] << "\t\t";
        cout << M[*it]['#'] << '\n';
    }
    cout << '\n';
}

void print_st(vector<char> st) {
    for (int i = 0; i < (int) st.size(); i++) {
        cout << st[i];
    }
    cout << '\n';
}

void Puts(int num, vector<char> &sta, int top, const string &str, int pos, string &info) {
    cout << "               " << num << "\t\t\t";
    for (int i = 0; i <= top; i++) cout << sta[i];
    cout << "\t\t\t";
    string cur = str.substr(pos);
    cout  << setw(15) << cur;
//    for (int i = pos; i < (int) str.size(); i++) cout << std::left<<setw(10) << str[i];
    cout << "\t\t\t";
    cout << info << '\n';
}

void check(const string &str) {
    vector<char> st(1000);
    st[0] = '#'; st[1] = start;
    int top = 1;
    int len = (int) str.size();
    int num = 1;
    string bk = "                                                       ";
    cout << "\n\n" << bk << "分析过程:" << '\n' << '\n';
    cout << "             " << "步骤" << "\t\t\t" << "分析栈" << "\t\t\t\t" << "剩余输入串" << "        " << "推导所用产生式或匹配" << '\n';
    for (int i = 0; i < len;) {
        string info;
        if (st[top] == str[i]) {
            if (str[i] != '#') {
                info += "“"; info += str[i]; info += "”匹配";
            } else {
                info = "接受";
            }
            Puts(num++, st, top, str, i, info);
            i++; top--;
            continue;
        }
        string next = M[st[top]][str[i]];
        if (next == "") {
            cout << "出错" << "\n";
            exit(0);
        } else if (next == "null") {
            info += st[top--]; info += " -> "; info += next;
            Puts(num++, st, top + 1, str, i, info);
        } else {
            info += st[top--]; info += " -> "; info += next;
            Puts(num++, st, top + 1, str, i, info);
            int l = (int) next.size();
            for (int j = l - 1; j >= 0; j--) {
                st[++top] = next[j];
            }
        }
    }
    cout << '\n';
}

int main() {
    init();
    get_nul();
    get_first();
    get_follow();
    get_select();
    get_M();
    cout << "                                               请输入要分析的符号串: ";
    string str;
    cin >> str;
    check(str);
    return 0;
}

//i+i*i#



//
//E TG
//G +TG
//G null
//T FH
//H *FH
//H null
//F i
//F (E)

//S AB
//S bC
//A null
//A b
//B null
//B aD
//C AD
//C b
//D aS
//D c


//
////
////  main.cpp
////  LL(1)
////
////  Created by Dousir9 on 2020/12/7.
////  Copyright © 2020 🍓XJK. All rights reserved.
////
//
//#include <iostream>
//#include <set>
//#include <map>
//#include <queue>
//#include <string>
//#include <fstream>
//#include <vector>
//#include <iomanip>
//using namespace std;
//
//map<char, set<string>> p; //产生式
//map<char, set<pair<string, set<char>>>> select; //select集
//map<char, int> nul; //是否可以推导出空
//map<char, set<char>> first, follow, includ; //first集、follow集
//map<char, int> done; //求first集是标记是否已经处理完
//set<char> vn, vt; //非终结符和终结符
//map<char, map<char, string>> M; //预测分析表
//char start; //开始符
//const char N = '@';
//const string blank = "                                                     ";
//
//void init() {
//    ifstream OutFile;
//    OutFile.open("/Users/xumingfei/Desktop/项目/编译原理/LL(1)/production.txt");
//    char c;
//    string str;
//    int f = 0;
//    while (!OutFile.eof()) {
//        OutFile >> c >> str;
//        if (!f) {f = 1; start = c;}
//        p[c].insert(str);
//        vn.insert(c);
//    }
//}
//
//inline bool in_vt(char c) {
//    if (vt.find(c) != vt.end()) return true;
//    //    if (!(c >= 'A' && c <= 'Z')) return true;
//    return false;
//}
//
//inline bool is_low(char c) {
//    if (!(c >= 'A' && c <= 'Z')) return true;
//    return false;
//}
//
//inline bool is_vn(char c) {
//    if (c >= 'A' && c <= 'Z') return true;
//    return false;
//}
//
//bool check_has_vt(const string &now) {
//    int len = (int) now.size();
//    for (int i = 0; i < len; i++)
//        if (in_vt(now[i])) return true;
//    return false;
//}
//
//int all_null(const string &now) {
//    if (now == "null") return true;
//    int len = (int) now.size();
//    int f = 0;
//    for (int i = 0; i < len; i++)
//        if (nul[now[i]] == 0) {
//            return 0;
//        } else if (nul[now[i]] == -1) {
//            f = 1;
//        }
//    if (f) return -1;
//    return 1;
//}
//
//void get_nul() {
//    map<char, set<string>> cur(p); //复制一份产生式(因为涉及到删除，所以为了保护原产生式，在复制的这份上进行删除操作)
//    set<char> now; //存放状态未定的非终结符
//    for (auto it = cur.begin(); it != cur.end(); ++it) {
//        nul[it->first] = -1; //状态置为待定
//        for (auto i = it->second.begin(); i != it->second.end();) {
//            string t = *i;
//            if (t != "null") for (int j = 0, len = (int) t.size(); j < len; j++)
//                if (is_low(t[j])) vt.insert(t[j]);
//            if (*i == "null") {
//                /*
//                 该非终结符能推导出空串，将该非终结符的标志置为1，并删除该非终结符的所有产生式
//                 此处不进行删除操作，而是用now存放那些状态未定的非终结符，后续中只去操作now中的元素，的这样该非终结符也就变相被”删除“了
//                 */
//                nul[it->first] = 1;
//                break;
//            } else if (check_has_vt(*i)) {
//                //若产生式中存在终结符，则删除该产生式
//                auto d = i;
//                ++i; //指针先后移，再删除
//                it->second.erase(*d);
//                continue;
//            } else {
//                //若全为非终结符，则保留该产生式
//                ++i;
//            }
//        }
//        if ((int) it->second.size() > 0 && nul[it->first] != 1) {
//            // 如果该非终结符还存在右部为全为非终结符的产生式并且状态不为1，则加入now中进行后续处理
//            now.insert(it->first);
//        } else if (nul[it->first] == -1) {
//            // 如果该非终结符不存在右部为全为非终结符的产生式并且状态为待定，则它无法推导为空串，置为0
//            nul[it->first] = 0;
//        }
//    }
//    string bk = "                                               ";
//    cout << "\n\n" << bk << "一次扫描后剩下的产生式:" << '\n' << '\n';
//    for (auto it = cur.begin(); it != cur.end(); ++it) {
//        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
//            cout << blank << it->first << " -> "<< *i << '\n';
//        }
//    }
//    bk = "                                                 ";
//    cout << "\n\n" << bk << "一次扫描后状态:" << '\n' << '\n';
//    for (auto it = nul.begin(); it != nul.end(); ++it) {
//        cout << blank << it->first << ": " << it->second << '\n';
//    }
//    cout << '\n';
//    int flag; //用于记录特征有无变化
//    do {
//        flag = 0; //刚开始时置为无变化
//        for (auto it = now.begin(); it != now.end();) {
//            int f = 0; //用于判断now中是否应该删除该非终结符
//            for (auto i = cur[*it].begin(); i != cur[*it].end();) {
//                int check = all_null(*i);
//                if (check == 1) {
//                    // 若产生式右部的非终结符全都能推导出空串，则该非终结符能推导出空
//                    nul[*it] = 1;
//                    flag = f = 1; // (1)有变化 (2)在now中删除该非终结符(删除该非终结的所有产生式)
//                    break;
//                } else if (check == 0) {
//                    // 若产生式右部的非终结符有的不能推导为空，则删除该产生式
//                    f = 1; // 在now中删除该非终结符(删除该非终结的所有产生式)
//                    auto d = i;
//                    ++i;
//                    cur[*it].erase(d);
//                } else {
//                    // 若产生式右部的非终结符还处于待定状态，则暂时不管
//                    ++i;
//                }
//            }
//            // 如果没有产生式了并且该非终结的状态还是待定，则将该非终结的状态置为0并且从now中删除
//            if ((int) cur[*it].size() == 0 && nul[*it] == -1) {
//                nul[*it] = 0;
//                f = 1;
//            }
//            if (f) {
//                // 该非终结符能推导出空或者不能推导出空，则now中删除该非终结符
//                auto d = it;
//                ++it;
//                now.erase(*d);
//            } else {
//                ++it;
//            }
//        }
//    } while (flag);
//    bk = "                                                    ";
//    cout << "\n" << bk << "最终状态:" << '\n' << '\n';
//    for (auto it = nul.begin(); it != nul.end(); ++it) {
//        cout << blank << it->first << ": " << it->second << '\n';
//    }
//}
//
//set<char> dfs(char now) {
//    if (in_vt(now)) { //如果是非终结符，则first集为本身
//        set<char> t;
//        t.insert(now);
//        return t;
//    } else {
//        if (done[now]) //如果该非终结符的first集已经处理过了，则直接返回即可，不必重复求解
//            return first[now];
//        for (auto it = p[now].begin(); it != p[now].end(); ++it) {
//            string r = *it;
//            if (r == "null") { //如果能推导出空串，则将空串加入first集
//                first[now].insert(N);
//            } else {
//                int len = (int) r.size();
//                for (int i = 0; i < len; i++) {
//                    if (is_vn(r[i])) { //若为非终结符，进行继续进行搜索，利用搜索结果更新first集
//                        auto h = dfs(r[i]); //dfs搜索该非终结符的first集
//                        if (nul[r[i]] && (h.find(N) != h.end())) h.erase(N); //如果有空串，先剔除空串
//                        for (auto j = h.begin(); j != h.end(); ++j) first[now].insert(*j); //补充first集
//                        if (nul[r[i]] != 1) { //如果当前字母不能推导为空串，则跳出，即没有空串加入
//                            break;
//                        } else if (i == len - 1) { //如果当前字母能推导为空串，且是最后一个，则在fisrt集中加上空串
//                            first[now].insert(N);
//                        }
//                    } else { //若为终结符，则first集就是它自身
//                        first[now].insert(r[i]);
//                        break;
//                    }
//                }
//            }
//        }
//        done[now] = 1; // 打上标记，表示该非终结符的first集已经处理过了，下次使用时直接返回即可，不必重复求解
//        return first[now];
//    }
//}
//
//void get_first() {
//    for (auto it = vn.begin(); it != vn.end(); ++it) dfs(*it); //对每个非终结符进行dfs
//    string bk = "                                                    ";
//    cout << "\n\n" << bk << "First集为:" << '\n' << '\n';
//    for (auto it = first.begin(); it != first.end(); ++it) {
//        cout << blank << "FIRST(" << it->first << ") = {";
//        for (auto j = it->second.begin(); j != it->second.end();) {
//            cout << *j++;
//            if (j != it->second.end()) cout << ",";
//        }
//        cout << "} \n";
//
//    }
//}
//
//void get_follow() {
//    follow[start].insert('#'); //把’#‘加入文法开始符号的Follow集中
//    for (auto it = p.begin(); it != p.end(); ++it) {
//        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
//            string r = *i;
//            int len = (int) r.size();
//            for (int i = 0; i < len; i++) {
//                if (is_vn(r[i]) && i != len - 1) {
//                    if (is_vn(r[i + 1])) { //右边为非终结符
//                        //follow集中加入右边为非终结符first中的非空元素
//                        for (auto it = first[r[i + 1]].begin(); it != first[r[i + 1]].end(); ++it)
//                            if (*it != N) follow[r[i]].insert(*it);
//                        //如果右边的非终结符能推导出空且为产生式最后一个元素，则加上产生式左边的非终结符的follow集
//                        if (nul[r[i + 1]] && i + 1 == len - 1) includ[r[i]].insert(it->first);
//                    } else {
//                        follow[r[i]].insert(r[i + 1]); //右边的为终结符，直接将它加到follow集中
//                    }
//                } else if (is_vn(r[i])) {
//                    includ[r[i]].insert(it->first);
//                }
//            }
//        }
//    }
//    string bk = "                                                    ";
//    cout << "\n\n" << bk << "includ为:" << '\n' << '\n';
//    for (auto it = includ.begin(); it != includ.end(); ++it) {
//        cout << blank << it->first << ":{ ";
//        for (auto j = it->second.begin(); j != it->second.end(); ++j) {
//            cout << *j << " ";
//        }
//        cout << "} \n";
//    }
//    int flag;
//    do {
//        flag = 0;
//        for (auto it = includ.begin(); it != includ.end(); ++it) {
//            int cur = (int) follow[it->first].size();
//            for (auto i = it->second.begin(); i != it->second.end(); ++i) {
//                for (auto j = follow[*i].begin(); j != follow[*i].end(); ++j) {
//                    follow[it->first].insert(*j);
//                }
//            }
//            if ((int) follow[it->first].size() > cur) flag = 1;
//        }
//    } while (flag);
//    cout << "\n\n" << bk << "Follow集为:" << '\n' << '\n';
//    for (auto it = follow.begin(); it != follow.end(); ++it) {
//        cout << blank << "FOLLOW(" << it->first << ") = {";
//        for (auto j = it->second.begin(); j != it->second.end();) {
//            cout << *j++;
//            if (j != it->second.end()) cout << ",";
//        }
//        cout << "} \n";
//    }
//}
//
//void get_select() {
//    for (auto it = p.begin(); it != p.end(); ++it) {
//        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
//            set<char> t;
//            string r = *i;
//            if (r == "null") {
//                for (auto l = follow[it->first].begin(); l != follow[it->first].end(); ++l)
//                    t.insert(*l);
//                select[it->first].insert({r, t});
//                continue;
//            }
//            int len = (int) r.size();
//            for (int j = 0; j < len; j++) {
//                if (is_vn(r[j])) {
//                    for (auto k = first[r[j]].begin(); k != first[r[j]].end(); ++k)
//                        if (*k != N) t.insert(*k);
//                    if (nul[r[j]]) {
//                        if (j == len - 1)
//                            for (auto l = follow[it->first].begin(); l != follow[it->first].end(); ++l)
//                                t.insert(*l);
//                    } else {
//                        break;
//                    }
//                } else {
//                    t.insert(r[j]);
//                    break;
//                }
//            }
//            select[it->first].insert({r, t});
//        }
//    }
//    string bk = "                                                    ";
//    cout << "\n\n" << bk << "Select集为:" << '\n' << '\n';
//    for (auto it = select.begin(); it != select.end(); ++it) {
//        for (auto i = it->second.begin(); i != it->second.end(); ++i) {
//            cout << blank << "SELECT(" << it->first << "->" << i->first << ") = {";
//            for (auto j = i->second.begin(); j != i->second.end();) {
//                cout << *j++;
//                if (j != i->second.end()) cout << ",";
//            }
//            cout << "}\n";
//        }
//    }
//}
//
//void get_M() {
//    for (auto it = select.begin(); it != select.end(); ++it)
//        for (auto i = it->second.begin(); i != it->second.end(); ++i)
//            for (auto j = i->second.begin(); j != i->second.end(); ++j)
//                M[it->first][*j] = i->first;
//    string bk = "                                                    ", bkk = "       ";
//    cout << "\n\n" << bk << "预测分析表为:" << '\n' << '\n';
//    cout << "         " << "\t\t";
//    for (auto it = vt.begin(); it != vt.end(); ++it)
//        cout << *it << "\t\t";
//    cout << "#" << '\n';
//    for (auto it = vn.begin(); it != vn.end(); ++it) {
//        cout << "         " << *it << "\t\t";
//        for (auto i = vt.begin(); i != vt.end(); ++i)
//            cout << M[*it][*i] << "\t\t";
//        cout << M[*it]['#'] << '\n';
//    }
//    cout << '\n';
//}
//
//void print_st(vector<char> st) {
//    for (int i = 0; i < (int) st.size(); i++) {
//        cout << st[i];
//    }
//    cout << '\n';
//}
//
//void Puts(int num, vector<char> &sta, int top, const string &str, int pos, string &info) {
//    cout << "               " << num << "\t\t\t";
//    for (int i = 0; i <= top; i++) cout << sta[i];
//    cout << "\t\t\t";
//    string cur = str.substr(pos);
//    cout  << setw(15) << cur;
//    //    for (int i = pos; i < (int) str.size(); i++) cout << std::left<<setw(10) << str[i];
//    cout << "\t\t\t";
//    cout << info << '\n';
//}
//
//void check(const string &str) {
//    vector<char> st(1000);
//    st[0] = '#'; st[1] = start;
//    int top = 1;
//    int len = (int) str.size();
//    int num = 1;
//    string bk = "                                                       ";
//    cout << "\n\n" << bk << "分析过程:" << '\n' << '\n';
//    cout << "             " << "步骤" << "\t\t\t" << "分析栈" << "\t\t\t\t" << "剩余输入串" << "        " << "推导所用产生式或匹配" << '\n';
//    for (int i = 0; i < len;) {
//        string info;
//        if (st[top] == str[i]) {
//            if (str[i] != '#') {
//                info += "“"; info += str[i]; info += "”匹配";
//            } else {
//                info = "接受";
//            }
//            Puts(num++, st, top, str, i, info);
//            i++; top--;
//            continue;
//        }
//        string next = M[st[top]][str[i]];
//        if (next == "") {
//            cout << "出错" << "\n";
//            exit(0);
//        } else if (next == "null") {
//            info += st[top--]; info += " -> "; info += next;
//            Puts(num++, st, top + 1, str, i, info);
//        } else {
//            info += st[top--]; info += " -> "; info += next;
//            Puts(num++, st, top + 1, str, i, info);
//            int l = (int) next.size();
//            for (int j = l - 1; j >= 0; j--) {
//                st[++top] = next[j];
//            }
//        }
//    }
//    cout << '\n';
//}
//
//int main() {
//    init();
//    get_nul();
//    get_first();
//    get_follow();
//    get_select();
//    get_M();
//    check("i+i*i#");
//    return 0;
//}
//
//
//
//
//
////
////E TG
////G +TG
////G null
////T FH
////H *FH
////H null
////F i
////F (E)
//
////S AB
////S bC
////A null
////A b
////B null
////B aD
////C AD
////C b
////D aS
////D c
