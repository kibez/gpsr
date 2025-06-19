// ============================================================================
// boost_gregorian_stub.cpp - ������ �������� ��� boost::gregorian
// ��� �������� � C++ Builder 6.0 �� RAD Studio 10.3.3
// ============================================================================
#include <string>

// ���������� ������������� ������� �� Boost.DateTime
namespace boost {
    namespace gregorian {
        
        // �������� ��� greg_month
        class greg_month {
        private:
            int month_;
        public:
            greg_month(int month) : month_(month) {}
            
            // ���������� as_short_string
            std::string as_short_string() const {
                static const char* months[] = {
                    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                };
                return (month_ >= 1 && month_ <= 12) ? months[month_] : "";
            }
            
            // ���������� as_long_string  
            std::string as_long_string() const {
                static const char* months[] = {
                    "", "January", "February", "March", "April", "May", "June",
                    "July", "August", "September", "October", "November", "December"
                };
                return (month_ >= 1 && month_ <= 12) ? months[month_] : "";
            }
            
            int as_number() const { return month_; }
        };
        
        // ���� ����� ������ ������� greg_month, ���������� �����
    }
}