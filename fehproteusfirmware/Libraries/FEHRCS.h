#ifndef FEHRCS_H
#define FEHRCS_H

#include <FEHXBee.h>

/**
 * @brief Access to RCS functionality
 *
 * Your Proteus will show a blue light indicator when it is in range of
 * RCS.<br/> You must first call InitializeTouchMenu() before RCS functionality
 * will work.
 *
 */
class FEHRCS {
 public:
  /**
   * @brief Setup system used to select RCS course
   *
   * You must be in range of the course to be able to successfully initialize.
   *
   */
  void InitializeTouchMenu(const char* team_key);

  /**
   * @brief Get course number corresponding to current region
   *
   * Get course number corresponding to current region
   *
   * @return unsigned char 1 (regions A-D), 2 (regions E-H), 3 (regions I-L)
   */
  unsigned char CurrentCourse();

  /**
   * @brief Get current region's corresponding letter
   *
   * Get current region's corresponding letter from
   * set { A, B, C, D, E, F, G, H, I, J, K, L }
   *
   * @return char Region letter RCS was initialized to
   */
  char CurrentRegionLetter();

  /**
   * @brief Get time remaining for a match
   *
   * Begins transmitting time remaining with 90 seconds left in the match.
   *
   * @return int Number of seconds remaining
   */
  int Time();

  // Objective functions:
  /**
   * @brief Get correct lever for task
   *
   * Get correct lever for task
   *
   * @return int 0 (left lever), 1 (middle lever), 2 (right lever)
   */
  int GetLever();

  /**
   * @brief Tells you if a lever has been flipped down but not up
   *
   * @return int 1 if lever has been flipped down, but not up. 0 otherwise
   */
  int isLeverFlipped();

  /**
   * @brief Tells you if the window is fully open or closed
   *
   * @return int 1 if window is fully open, 0 if closed
   */
  int isWindowOpen();

  // returns the number of the current course { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  // 10, 11 }
  int CurrentRegion();

  FEHRCS();

 private:
  // RCS debug/deprecated functions:
  unsigned char WaitForPacket();
  int WaitForPacketDebug(int* packetsFound,
                         int* packetsLost,
                         int* lastFoundPacketTime);

  // Manually pick and configure a region
  // int region => { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }
  // char region => { a, b, c, d, e, f, g, h, i, j, k, l } || { A, B, C, D, E,
  // F, G, H, I, J, K, L }
  void Initialize(int region, const char* team_key);
  void Initialize(char region, const char* team_key);

  FEHXBee _xbee;
};

/**
 * @brief Global access to FEHRCS class
 *
 * Global access to FEHRCS class
 *
 */
extern FEHRCS RCS;

#endif  // FEHRCS_H