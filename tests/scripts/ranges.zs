void test()
{
  int x = 5;

  switch (x) {
  case 0..5:
    printf("ERROR\n");
    break;
  case 5..10:
    printf("ERROR\n");
    break;
  default:
    printf("SUCCESS\n");
    break;
  }

  switch (x) {
  case 0=..5:
    printf("ERROR\n");
    break;
  case 5 =..10:
    printf("SUCCESS\n");
    break;
  default:
    printf("ERROR\n");
    break;
  }

  switch (x) {
  case 5..=10:
    printf("ERROR\n");
    break;
  case 0..=5:
    printf("SUCCESS\n");
    break;
  default:
    printf("ERROR\n");
    break;
  }

  switch (x) {
  case 0 =..=5:
    printf("SUCCESS\n");
    break;
  case 5 =..=10:
    printf("ERROR\n"); // because a higher case in this switch should've already
                       // matched
    break;
  default:
    printf("ERROR\n");
    break;
  }
}
