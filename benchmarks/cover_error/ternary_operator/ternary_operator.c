extern char __VERIFIER_nondet_char();

void reach_error() { return; }

int main() {

    char a = __VERIFIER_nondet_char();
    char b = __VERIFIER_nondet_char();
    char c = __VERIFIER_nondet_char();

    (a == 'a' && b == 5 && c == 16) ? reach_error() : (void) 0;
}
