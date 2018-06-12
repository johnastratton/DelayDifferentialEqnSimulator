#include "csvr.hpp"
#include "util/color.hpp"

#include <cfloat> // For FLT_MAX as an internal error code
#include <cmath>
#include <limits>
#include <iostream>


csvr::csvr(std::string const& file_name, bool suppress_file_not_found) :
    iFile(file_name), iLine(1)
{
    if (!iFile.is_open() && !suppress_file_not_found)
        std::cout << color::set(color::RED) << "CSV file input failed. CSV file \'" <<
            file_name << "\' not found or open." << color::clear() << '\n';
}

bool csvr::is_open() const {
    return iFile.is_open();
}

bool csvr::get_next() {
    return csvr::get_next(static_cast<RATETYPE *>(nullptr));
}

bool csvr::get_next(int* rate) {
  RATETYPE result;
  bool success = get_next(&result);
  if (success && rate) *rate = std::round(result);
  return success;
}

bool csvr::get_next(RATETYPE* pnRate) {
  // Only bother if open
  if (!iFile.is_open()) {
      std::cout << color::set(color::RED) << "CSV parsing failed. "
          "No CSV file found/open." << color::clear() << '\n';
      return false;
  }
  // tParam data from file to be "pushed" to pfRate
  std::string tParam;

  char c;
  while (iFile >> c) {
      // Skip line comments
      if (c == '#') {
          iFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          tParam.clear();
          ++iLine;
      }
      // Parse only if not whitespace except for \n
      else if (c != ' ' && c != '\t')
      {
        // Ignore non-numeric, non data seperator, non e, -, and + characters
        if (!( (c >= '0' && c <= '9') || c == '.' || c == ',' || c == '\n' ||
                c == 'e' || c == '-' || c == '+') )
        {
            iFile.ignore(std::numeric_limits<std::streamsize>::max(), ',');
            tParam.clear();
        }
        // If hit data seperator, add data to respective array
        // '\n' is there in case there is no comma after last item
        else if (c != ',' && c != '\n') {
          tParam += c;
        }
        else if (!tParam.empty()) {
          char* tInvalidAt;
          RATETYPE tRate = strtold(tParam.c_str(), &tInvalidAt);

          // If found invalid while parsing
          if (*tInvalidAt)
          {
              std::cout << color::set(color::RED) <<
                  "CSV parsing failed. Invalid data contained "
                  "at line " << iLine << "." <<
                  color::clear() << '\n';
              return false;
          }
          // Else was success
          if (pnRate) { *pnRate = tRate; }
          return true;
        }
      }

      // Increment line counter
      if (c == '\n') {
          ++iLine;
      }
  }

  // End of file
  return false;
}
