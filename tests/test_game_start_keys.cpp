/* GPL-3.0-or-later. Match RAD's C++ keyboard-scanner linkage in host tests. */
extern "C" void radwolf_test_check_menu(void);
void readKeyDoom(unsigned int *events, unsigned char *count)
{
    (void)events;
    *count = 0;
    radwolf_test_check_menu();
}
