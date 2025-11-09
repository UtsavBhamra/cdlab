// Include necessary C++ standard library headers
#include <iostream>  // For input/output operations
#include <vector>    // For dynamic arrays
#include <map>       // For key-value pair storage
#include <set>       // For storing unique elements
#include <string>    // For string operations
using namespace std;

// Global data structures
vector<pair<string, string>> g;  // Grammar: stores (LHS, RHS) pairs for each production
set<char> S;                      // Set of all symbols (terminals and non-terminals) 

// Creates an LR(0) item string representation
// An item is a production with a dot indicating the current position
// Example: A->α·β means we've seen α and expect β next
// Parameters: A = LHS, B = RHS, d = dot position
string make(string A, string B, int d) {
    return A + "->" + B.substr(0, d) + "." + B.substr(d);
    // Returns: "A->α.β" where α = B[0..d-1] and β = B[d..end]
}

// Parses an item string to extract its components
// Input: "A->α.β" (item string)
// Output: A (LHS), B (RHS without dot), d (dot position)
void parse(string it, string &A, string &B, int &d) {
    int x = it.find("->");        // Find the arrow position
    A = it.substr(0, x);          // Extract LHS (everything before "->")
    B = it.substr(x + 2);         // Extract RHS with dot (everything after "->")
    d = B.find('.');              // Find the dot position in RHS
    B.erase(d, 1);                // Remove the dot from RHS to get clean production
}

// Computes the CLOSURE of a set of LR(0) items
// If [A -> α·Bβ] is in I and B is a non-terminal,
// add all items [B -> ·γ] for each production B -> γ
set<string> closure(set<string> I) {
    bool c;  // Flag to track if new items were added
    do {
        c = false;  // Assume no changes in this iteration
        
        // Convert set to vector to safely iterate (avoiding iterator invalidation)
        vector<string> v(I.begin(), I.end());
        
        // For each item in the current set
        for (auto it : v) {
            string A, B;  // LHS and RHS of the item
            int d;        // Dot position
            parse(it, A, B, d);  // Parse the item to get components
            
            // Check if dot is before a non-terminal symbol
            if (d < (int)B.size() && isupper(B[d])) {
                // B[d] is a non-terminal, so add all productions of B[d]
                for (auto p : g) {
                    // If this production's LHS matches the non-terminal after dot
                    if (p.first[0] == B[d]) {
                        // Add item [B[d] -> ·production] with dot at position 0
                        if (I.insert(make(p.first, p.second, 0)).second)
                            c = true;  // Set flag if a new item was added
                    }
                }
            }
        }
    } while (c);  // Repeat until no new items are added (fixed point)
    return I;
}

// Computes GOTO(I, X) - the transition from item set I on symbol X
// GOTO(I, X) = closure of all items [A -> αX·β] where [A -> α·Xβ] is in I
set<string> goTo(set<string> I, char X) {
    set<string> J;  // Temporary set to store moved items
    
    // For each item in the input set I
    for (auto it : I) {
        string A, B;  // LHS and RHS of the item
        int d;        // Dot position
        parse(it, A, B, d);  // Parse the item
        
        // Check if dot is before symbol X
        if (d < (int)B.size() && B[d] == X)
            // Move the dot one position right (after consuming X)
            J.insert(make(A, B, d + 1));
    }
    
    // If no items were moved, return empty set
    // Otherwise, return closure of the moved items
    return J.empty() ? set<string>{} : closure(J);
}

int main() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;  // Read number of productions
    
    string s;
    getline(cin, s);  // Consume the newline after the number
    
    cout << "Enter productions (A=alpha|beta|..., use # for epsilon):\n";
    
    // Read and parse each production
    for (int i = 0; i < n; i++) {
        getline(cin, s);  // Read full line (e.g., "S=AB|a")
        
        int eq = s.find('=');  // Find the '=' separator
        string lhs = s.substr(0, eq);      // Extract LHS (left of '=')
        string rhs = s.substr(eq + 1);     // Extract RHS (right of '=')
        string prod = "";                   // Temporary string to build each production
        
        // Split RHS by '|' to handle multiple alternatives
        for (char c : rhs) {
            if (c == '|') {
                // End of one alternative, add it to grammar
                g.push_back({lhs, prod == "#" ? "" : prod});  // Replace '#' with empty string (epsilon)
                
                // Add all symbols in this production to symbol set S
                for (char x : prod) S.insert(x);
                
                prod = "";  // Reset for next alternative
            } else {
                prod += c;  // Build current production
            }
        }
        
        // Add the last production (after the last '|' or if no '|' exists)
        g.push_back({lhs, prod == "#" ? "" : prod});
        
        // Add symbols from last production to symbol set
        for (char x : prod) S.insert(x);
        
        // Add LHS non-terminal to symbol set
        S.insert(lhs[0]);
    }

    // Augment the grammar with a new start symbol
    // For grammar with start S, add production S' -> S
    string start = g[0].first;  // Original start symbol (LHS of first production)
    string aug = start + "'";   // Augmented start symbol (S')
    g.insert(g.begin(), {aug, start});  // Insert S' -> S at the beginning
    S.insert(start[0]);  // Add start symbol to symbol set

    // Create initial item set I0: closure of [S' -> ·S]
    set<string> I0 = closure({make(aug, start, 0)});
    
    // Data structures for canonical collection
    vector<set<string>> C = {I0};     // C: collection of all item sets (states)
    vector<map<char, int>> T(1);      // T: transition table T[state][symbol] = next_state
    map<string, int> idx;             // idx: maps item set to its index in C

    // Lambda function to create a unique key for an item set
    // Concatenates all items with '|' separator
    auto key = [](set<string> s) {
        string k;
        for (auto &x : s) k += x + "|";
        return k;
    };
    
    idx[key(I0)] = 0;  // Map I0 to index 0

    // Build the canonical collection of LR(0) items
    // Process each state and compute GOTO for all symbols
    for (int i = 0; i < (int)C.size(); i++) {
        // For each symbol in the grammar
        for (char X : S) {
            // Compute GOTO(C[i], X)
            set<string> J = goTo(C[i], X);
            
            if (J.empty()) continue;  // Skip if no transition on X
            
            string k = key(J);  // Create unique key for this item set
            
            // Check if this item set is new
            if (!idx.count(k)) {
                // New state found
                idx[k] = C.size();     // Assign it the next index
                C.push_back(J);        // Add to collection
                T.push_back({});       // Add empty transition map for this state
            }
            
            // Record the transition: from state i on symbol X go to state idx[k]
            T[i][X] = idx[k];
        }
    }

    // Print the canonical collection of LR(0) item sets
    cout << "\nCanonical LR(0) Item Sets:\n";
    
    // For each state in the collection
    for (int i = 0; i < (int)C.size(); i++) {
        cout << "I" << i << ":\n";  // Print state number
        
        // Print all items in this state
        for (auto it : C[i]) 
            cout << it << "\n";
        
        // Print all transitions from this state
        for (auto p : T[i]) 
            cout << "GOTO(I" << i << "," << p.first << ")=I" << p.second << "\n";
        
        cout << "\n";  // Blank line between states
    }
}