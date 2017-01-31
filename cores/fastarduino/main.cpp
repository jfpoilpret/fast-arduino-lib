
#define UNUSED __attribute__((unused))

int main() __attribute__((weak));
int main()
{
	return 0;
}

void exit(int status) __attribute__((weak));
void exit(int status UNUSED)
{
}
