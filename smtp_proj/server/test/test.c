#include "CUnit/Basic.h"
#include "pattern.h"

patterns_t patterns;

int init_pcre(void)
{
   if (pattern_init() != 0)
      return -1;
   return 0;
}

int clean_pcre(void)
{
   return 0;
}

void testQUIT(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_QUIT, "QUIT", &content));
  CU_ASSERT(-1 == pattern_compute(PT_QUIT, "EXIT", &content));
  CU_ASSERT(-1 == pattern_compute(PT_QUIT, "QUIT HELO", &content));
}

void testRSET(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_RSET, "RSET", &content));
  CU_ASSERT(-1 == pattern_compute(PT_RSET, "RESET", &content));
  CU_ASSERT(-1 == pattern_compute(PT_RSET, "RSET HELO", &content));
}

void testVRFY(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_VRFY, "VRFY mail@example.com", &content));
  CU_ASSERT_STRING_EQUAL(content, "mail@example.com");
  CU_ASSERT(-1 == pattern_compute(PT_VRFY, "VERIFY", &content));
  CU_ASSERT(-1 == pattern_compute(PT_VRFY, "VRFY", &content));
}

void testHELO(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_HELO, "HELO example.com", &content));
  CU_ASSERT_STRING_EQUAL(content, "example.com");
  CU_ASSERT(-1 == pattern_compute(PT_HELO, "EHLO", &content));
  CU_ASSERT(-1 == pattern_compute(PT_HELO, "HELO", &content));
}

void testEHLO(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_EHLO, "EHLO example.com", &content));
  CU_ASSERT_STRING_EQUAL(content, "example.com");
  CU_ASSERT(-1 == pattern_compute(PT_EHLO, "HELO", &content));
  CU_ASSERT(-1 == pattern_compute(PT_EHLO, "EHLO", &content));
}

void testMAIL(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_MAIL, "MAIL FROM: mail@example.com", &content));
  CU_ASSERT_STRING_EQUAL(content, "mail@example.com");
  CU_ASSERT(-1 == pattern_compute(PT_MAIL, "MAIL FROM mail@example.com", &content));
  CU_ASSERT(-1 == pattern_compute(PT_MAIL, "MAIL: mail@example.com", &content));
  CU_ASSERT(-1 == pattern_compute(PT_MAIL, "MAIL FROM:", &content));
}

void testRCPT(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_RCPT, "RCPT TO: mail@example.com", &content));
  CU_ASSERT_STRING_EQUAL(content, "mail@example.com");
  CU_ASSERT(-1 == pattern_compute(PT_RCPT, "RCPT TO mail@example.com", &content));
  CU_ASSERT(-1 == pattern_compute(PT_RCPT, "RCPT: mail@example.com", &content));
  CU_ASSERT(-1 == pattern_compute(PT_RCPT, "RCPT TO:", &content));
}

void testDATA(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_DATA, "DATA", &content));
  CU_ASSERT(-1 == pattern_compute(PT_DATA, "DATA EHLO", &content));
}

void testDATA_END(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_DATA_END, ".", &content));
  CU_ASSERT(-1 == pattern_compute(PT_DATA_END, "END", &content));
}

void testEMAIL(void)
{
  const char* content;
  CU_ASSERT(0 == pattern_compute(PT_EMAIL, "mail@example.com", &content));
  CU_ASSERT(-1 == pattern_compute(PT_EMAIL, "nomail:example.com", &content));
}

int main()
{
   CU_pSuite pSuite = NULL;

   /* Инициализировать репозиторий тестов CUnit */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* Добавить кейс в репозиторий */
   pSuite = CU_add_suite("pcre_suite", init_pcre, clean_pcre);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Добавление тестовых функций */
   if ((NULL == CU_add_test(pSuite, "test command QUIT", testQUIT)) ||
       (NULL == CU_add_test(pSuite, "test command RSET", testRSET)) ||
       (NULL == CU_add_test(pSuite, "test command VRFY", testVRFY)) ||
       (NULL == CU_add_test(pSuite, "test command HELO", testHELO)) ||
       (NULL == CU_add_test(pSuite, "test command EHLO", testEHLO)) ||
       (NULL == CU_add_test(pSuite, "test command MAIL", testMAIL)) ||
       (NULL == CU_add_test(pSuite, "test command RCPT", testRCPT)) ||
       (NULL == CU_add_test(pSuite, "test command DATA", testDATA)) ||
       (NULL == CU_add_test(pSuite, "test helper DATA_END", testDATA_END)) ||
       (NULL == CU_add_test(pSuite, "test helper EMAIL", testEMAIL)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Запуск тестов */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}