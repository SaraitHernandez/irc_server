# How to run ALL test at once -> from main directory run: sh ./tests/run_all_tests.sh

# MessageBuffer
echo ">>Starting MessageBuffer tests..."
c++ -Wall -Wextra -Werror -std=c++98 -I include src/MessageBuffer.cpp tests/test_MessageBuffer/test_MessageBuffer.cpp -o tests/test_MessageBuffer/run_test_MessageBuffer
./tests/test_MessageBuffer/run_test_MessageBuffer

# Parser
echo "\n\n>>Starting Parser tests..."
c++ -Wall -Wextra -Werror -std=c++98 -I include src/Parser.cpp src/Utils.cpp tests/test_Parser/test_Parser.cpp -o tests/test_Parser/run_test_Parser
./tests/test_Parser/run_test_Parser

# Replies
echo "\n\n>>Starting Replies tests..."
c++ -Wall -Wextra -Werror -std=c++98 -Itests/test_Replies/include -Iinclude src/Replies.cpp tests/test_Replies/test_Replies.cpp -o tests/test_Replies/run_test_Replies
./tests/test_Replies/run_test_Replies
