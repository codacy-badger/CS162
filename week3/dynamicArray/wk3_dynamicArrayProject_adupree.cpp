/*******************************************************************************
* File: wk3_dynamicArrayProject_adupree.cpp
*
* Description: Prototype Saints Fitness application to allow employees to view,
*              keep, search and modify membership records. Membership records 
*              will be stored into the "saintsMembers.txt" file. Each row in 
*              this file will represent a member, and each column(tab delimited)
*              will represent an attribute. 
*
*              When the program runs, it first will load each member from the 
*              file into memory as a "Member" object. It will then create a 
*              dynamic array based on the number of current members + 20. These
*              20 extra blocks of memory will be used to add new members. Any 
*              open slots in our Member array will be represented by a NULL 
*              pointer. When deleting members we will take the slot where the 
*              member was and point it to NUll. 
*              
*              When adding members we will search for a NULL pointer and add 
*              the new member at that location. If no NULL pointers are found 
*              then we must resize the array. We do this by allocating memory
*              for the size of the old array + 20 and then copy the data over to
*              the new array and deleting the old array. 
*
*              To prevent duplicate entries of members we will use a Map to hold
*              member ID's as keys and pointers to the Member object as the 
*              value. 
*
*              To search for a members email address we will use a dynamic array
*              of Member pointers to iterate through the list until a match has
*              been found and then return the pointer of the member. 
*
* Author: Alexander DuPree
*
* Compiler:  GNU GCC 5.4.0
*
* Date: 2018-04-19
*
* Modifications:
*******************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "member.h"
#include "CSV.h"

void printInterface();

void printError(std::string error);

void printMembers(std::map<unsigned int, Member*>& IDs, std::string caption);
// Prints all members, sorted by ID

void printMember(Member* member, std::string caption);
// Prints a single members information

void resetInputStream();
// resets stream failed state, flushes input buffer

void pauseConsole();
// Prints "press enter to continue" and uses cin.get() to pause the program

void loadData(CSV::Reader& reader, Member members[]);
// Loads data from the input file into the Members array. 

void writeData(CSV::Writer& writer, std::map<unsigned int, Member*>& IDs,
        const std::string FIELDS[], const int COLUMNS);

void addMember(Member *& members, std::string fName, std::string lName,
        std::string email, int& size);
// TODO rework this function into seperate functions. A int that returns the 
// index of the free location. AddMember that adds the member into the location
// and a resize call for when the array is full

void deleteMember(std::map<unsigned int, Member*>& IDs,  int memberID);
// Looks in the map for a matching ID. If a matching ID is found then the 
// members values are set to null, signaling an empty block of memory for new 
// members to be added to. Else, prints to console that no ID was found. 

void resize(Member *& members, int& size);
// Allocates a new dynamic array of size + 10. Copies over the contents, deletes
// the old array and then points the old pointer to the new array. A pointer 
// reference is used to ensure the original pointer is being reassigned

int strToInt(const std::string& input);
// loops through validated input string and builds an integer by taking
// advantage of a encoding offset. i.e. the characters '0' - '9' are 48 - 57 on 
// the ascii and utf-8 tables. this means we can take each character from the 
// input string and minus 48 from it and get the represented integer.

unsigned int createID(const std::string& name);
// Utilizes a VERY basic hash function to create a unique 6 digit identification
// number. This hash function can cause collisions, however in the 
// scope of this program it is unlikely.

std::string intToStr(const int num);
// Uses string stream to return a string of the integer object

std::string toUpper(const std::string& name);
// Returns the uppercase version of parameter name

template <typename T>
T getInput(std::string prompt);
// grabs input from the keyboard. Discards all characters after the first space


Member* searchByName(std::map<unsigned int, Member*>& IDs, std::string fName, 
        std::string lName);

int main()
{   
    std::ofstream fout;

    int size = 10;
    // Initial size of 10 allows a buffer of 10 empty member slots in our 
    // dynamic array. This value is CHANGED everytime the array needs to resize
    const int COLUMNS = 4;
    // Columns represents the number of columns to be used in our txt file
    
    char input;
    const char DELIM = ',';
    const char FNAME[] = "saintsMembers.txt";
    const std::string FIELDS[COLUMNS] = {"ID", "FNAME", "LNAME", "EMAIL"};

    CSV::Reader reader(FNAME, DELIM);

    size += reader.lines();
    // Size is equal to the number of members in the FNAME file + 10

    Member* members = new Member[size];
    // Dynamic Array to hold all member objects with a buffer for adding new
    // members


    try
    {
        loadData(reader, members);
        // Catch invalid id error
    }
    catch (invalid_ID& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << FNAME << " contained invalid IDs" << std::endl;
        exit(1);
    }

    reader.closeFile();

    // Interface and output
    do {

        // Placeholder variables for grabbing input
        int id = 0;
        std::string fName = "", 
                    lName = "", 
                    email = "";


        printInterface();

        input = getInput<char>("\n> ");    

        switch(input)
        {
            // Add member
            case '1' : 

                fName = getInput<std::string>("\nEnter members First Name: ");
                lName = getInput<std::string>("Enter members Last Name: ");
                email = getInput<std::string>("Enter members email address: ");

                try 
                {
                    addMember(members, fName, lName, email, size);
                }
                catch (invalid_ID& err)
                {
                    std::cerr << "\nSorry, looks like '" << fName << ' ' <<lName
                              << "' is already in our system." << std::endl;
                }
                break;

            // Remove member
            case '2' : 

                id = getInput<int>("\nEnter the members ID to remove: ");
                deleteMember(members->memberIDs, id);
                break;

            // Search by name, change email
            case '3' : 
            {
                fName = getInput<std::string>("\nEnter members First Name:  ");
                lName = getInput<std::string>("Enter members Last Name: ");
                Member* member = searchByName(members->memberIDs, fName, lName);

                if (member)
                {
                    printMember(member, "\nMember found!");
                    std::cout << "Change email address? (Y/N)" << std::endl;

                    char choice = getInput<char>("> ");
                    if (choice == 'y' || choice == 'Y')
                    {
                        email = getInput<std::string>("Enter the new email: ");
                        member->setEmail(email);
                        printMember(member, "\nEmail changed!");
                    }
                }
                else
                {
                    std::cout << "\nMember not found: " << lName << " " << fName 
                              << std::endl; 
                }
            }   break;

            // Print all members
            case '4' :

                printMembers(members->memberIDs, "Saints Fitness Members: ");
                break;

            case 'q' : break;

            default : printError("Invalid input.");
        }

        pauseConsole();

    } while(input != 'q' && input != 'Q');
  
    CSV::Writer writer("test.txt", DELIM);
    // Instantiate writer object
    
    writeData(writer, members->memberIDs, FIELDS, COLUMNS);
    // Write changes to members to saintsMembers.txt
    
    return 0;
}

void printInterface()
{
    std::cout << "\n\t\tSAINTS FITNESS\nWelcome to Saints Fitness membership " 
              << "tracker.\n\nPlease choose an option:" << std::endl;
    std::cout << "\n  1.\tAdd a new member\n  2.\tRemove a member\n  3.\t"
              << "Search for a member\n  4.\tView all members!"
              << "\nPress 'q' to quit." << std::endl;
    return;
}

void printError(std::string error)
{
    std::cout << error << std::endl;
    return;
}

void printMembers(std::map<unsigned int, Member*>& IDs, std::string caption)
{
    std::map<unsigned int, Member*>::iterator it;

    std::cout << caption << std::endl;
    for (it = IDs.begin(); it != IDs.end(); it++)
    {
        std::cout << *it->second;
    }

    return;
}

void printMember(Member* member, std::string caption)
{
    std::cout << caption << std::endl; 
    std::cout << *member;
    return;
}

void resetInputStream()
{
    // reset failed state
    std::cin.clear();

    // discard characters to the limit of the stream size OR to newline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return;
}

void pauseConsole()
{
    std::cout << "\nPress enter to continue." << std::endl;
    std::cin.get();
    return;
}

void loadData(CSV::Reader& reader, Member members[])
{
    std::string* fields = new std::string[reader.fields()];

    reader.seek(1);
    // Skips the field names line
    
    for (int i = 0; i < reader.lines() - 1; i++)
    {
        reader.readLine(fields);
        members[i] = Member(strToInt(fields[0]), fields[1], fields[2], 
                            fields[3]);
        // throws custom exception invalid_ID, this exception is caught in main
    }
    
    delete [] fields;

    return;
}
void writeData(CSV::Writer& writer, std::map<unsigned int, Member*>& IDs,
        const std::string FIELDS[], const int COLUMNS)
{
    std::string data[COLUMNS];
    std::map<unsigned int, Member*>::iterator it;

    writer.writeLine(FIELDS, COLUMNS);

    for (it = IDs.begin(); it != IDs.end(); it++)
    {
        data[0] = intToStr(it->second->ID());
        data[1] = it->second->fname();
        data[2] = it->second->lname();
        data[3] = it->second->email();
        writer.writeLine(data, COLUMNS);
    }
    return;
}

void addMember(Member *& members, std::string fName, std::string lName,
        std::string email, int& size)
{
    for (int i = 0; i < size; i++)
    {
        // Find an empty slot in our array
        if (members[i].ID() == 0)
        {
            members[i] = Member(createID(fName + lName), fName, lName, email);
            printMember(&members[i], '\n'+members[i].name()+" was added!");
            return;
        }
    }
    // If the loop completes we need to resize the array and try again.
    resize(members, size);

    // Recurisvely call itself until member can be added.
    addMember(members, fName, lName, email, size);
    
    return;
}

void deleteMember(std::map<unsigned int, Member*>& IDs, int memberID)
{
    std::map<unsigned int, Member*>::iterator it;
    it = IDs.find(memberID);
    if (it == IDs.end())
    {
        printError("\nNo matching ID found");
    }
    else 
    {
        std::cout << "\nMember: " << memberID << "\t"  
                  << it->second->name() << " removed.";
        // Member with matching ID is assigned null values.
        *it->second = Member();
        IDs.erase(it);
        IDs.erase(0);
    } 

    return;
}

void resize(Member *& members, int& size)
{
    int newSize = size + 10;

    Member* resized = new Member[newSize];

    // Copy the contents from old to new array
    for (int i = 0; i < size; i++)
    {
        resized[i] = members[i];
    }
       
    size = newSize;

    // Delete the old array
    delete [] members;

    // Reassign the members pointer
    members = resized;

    return;

}

int strToInt(const std::string& input)
{
    // num will be used to build the integer
    int num = 0;
    int sign = 1;
    unsigned int i = 0;

    // store the sign of the input. then increment index to skip sign.
    if (input[0] == '-') 
    { 
        sign = -1; 
        i++;
    }
    for (; i <= input.length(); i++)
    {
        // drop decimal values
        if (input[i] < 48 || input[i] > 57) 
        { 
            break; 
        }
        
        // build integer from left to right. 
        num = 10 * num + (input[i] - '0');
    }
    return num * sign;
}

unsigned int createID(const std::string& name)
{
    std::string upperName = toUpper(name);
    unsigned long hash = 5381;
    // Prime number for hashing, long data type is used to hold the all digits
    // the has may reach
    
    for(unsigned int i = 0; i < upperName.size(); i++)
    {
        hash = 33 * hash + (unsigned char)upperName[i];
    }
    
    // Return the last 6 digits of the hash
    return static_cast<unsigned int>(hash % 1000000);
}

std::string intToStr(const int num)
{
    std::stringstream integer;
    integer << num;
    return integer.str();
}

std::string toUpper(const std::string& name)
{
    std::string upperCase = "";

    for(unsigned int i = 0; i < name.size(); i++)
    {
        if (name[i] >= 'a' && name[i] <= 'z')
        {
            upperCase += name[i] - 32;
        }
        else
        {
            upperCase += name[i];
        }

    }
    return upperCase;
}

template <typename T>
T getInput(std::string prompt)
{
    T input;

    std::cout << prompt;
    std::cin >> input;
    resetInputStream();

    return input;
}

Member* searchByName(std::map<unsigned int, Member*>& IDs, std::string fName, 
        std::string lName)
{
    std::map<unsigned int, Member*>::iterator it;

    it = IDs.find(createID(fName + lName));
    // Hashes on the name of the member to find the ID built on that hash
    if (it == IDs.end())
    {
        return NULL;
    }
    else
    {
        return it->second;
    }
}
