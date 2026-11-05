int main(void) {
  long res = -0x7F7E80B1L >> 4;

  if (res == -0x7F7E80CL)
    return 0; /* right shift is signed */

  /* see if unsigned-shift hack will fix it. */
  /* we can't just test exact value since it depends on width of long... */
  res |= 0xFFFFFFFFL << (32-4);
  if (res == -0x7F7E80CL)
    return 1; /* right shift is unsigned */

  return 2; /* fatal error */
}
