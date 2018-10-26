// 
// GENERAL STRUCTURE AND ORDERING (don't worry, I'm not angry I'm just capitalized)   
// 
//      Structure code in this order:
//      1. Declare variables (declare necessary inputs/information either in comments or code)
//      2. Declare functions (describe the functions operations step by step and denote each 
//                            function clearly with comments/spacing)
//      3. Declare main loop (attempt to structure your main with comments clearly denoting
//                            what sections do what and attempt to establish a clear running
//                            thought process to make sure people understand what's happening)
//
//      Notes: Try and treat your code as if it's meant to be interpretted by a computer AND a
//             human (specifically a human that can read English). English reading humans read
//             left to right, top to bottom so attempt to structure your code in way that flows
//             as such.
//
//      (Sample starts below dotted line)
// ----------------------------------------------------------------------------------------------

// _______________________________________________________________________________________
// 
// Title (Ex. code_template.cpp)
// Author/Authors (Ex. Mr. Wiggles)
// 
// Date Created (Ex. October 23, 2018)
// 
// Purpose/Use: (Ex. This code is to provide a template/sample for SeCon members to ensure
//                   clarity/familiarity for every team member.)
//
// _______________________________________________________________________________________

// Variables
// ___________________________________

int var_1;                  // Explain variables next to their declaration

// ___________________________________


// Functions
// ___________________________________

int function_1(int par_1) {     // Explain the function's purpose 
                            // and what parameters are what
    
    // Step 1
    // ______________
    par_1 = par_1 + 1;      // Explain step 1

    // Step 2
    // ______________
    par_1 = par_1 + 2;      // Explain step 2

    return par_1;
}

// ___________________________________


// Main Loop
// ___________________________________

main() {

    // Section 1
    // ______________
    var_1 = 0;                      // Explain Section 1

    // Section 2
    // ______________
    var_1 = function_1(var_1);      // Explain Section 2

    return;
}

// ___________________________________
