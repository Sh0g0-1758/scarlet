/* Make sure we can handle control characters that are in the source character set */

int main(void)
{
    int tab = ' ';
    int vertical_tab = '\v';
    int form_feed = '\f';
    if (tab != '\t') {
        return 1;
    }
    if (vertical_tab != '\v') {
        return 2;
    }

    if (form_feed != '\f') {
        return 3;
    }

    return 0;
}