#ifndef SEMANTIC_VERSION_H
#define SEMANTIC_VERSION_H

#include <cstdint>
#include <string>


/**
 * Semantic version object.
 *
 * Can easily convert between a string and itself.
 */
class SemanticVersion
{
public:
    /**
     * Create a new SemanticVersion object.
     *
     * @param versionString String that contains a version number, e.g.: "v1.5.23".
     */
    SemanticVersion(const std::string &versionString);

    /**
     * Convert the SemanticVersion object to a string.
     *
     * @return Semantic version string, e.g.: "v1.5.23".
     */
    std::string String();

    /**
     * Convert the SemanticVersion object to a char*.
     *
     * @return Semantic version string, e.g.: "v1.5.23".
     */
    const char *CString();

    // All of the comparison operators.
    friend bool operator==(const SemanticVersion &sv1, const SemanticVersion &sv2);
    friend bool operator!=(const SemanticVersion &sv1, const SemanticVersion &sv2);
    friend bool operator>(const SemanticVersion &sv1, const SemanticVersion &sv2);
    friend bool operator<(const SemanticVersion &sv1, const SemanticVersion &sv2);
    friend bool operator>=(const SemanticVersion &sv1, const SemanticVersion &sv2);
    friend bool operator<=(const SemanticVersion &sv1, const SemanticVersion &sv2);

private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
    std::string m_extra;
};

bool operator==(const SemanticVersion &sv1, const SemanticVersion &sv2);
bool operator!=(const SemanticVersion &sv1, const SemanticVersion &sv2);
bool operator>(const SemanticVersion &sv1, const SemanticVersion &sv2);
bool operator<(const SemanticVersion &sv1, const SemanticVersion &sv2);
bool operator>=(const SemanticVersion &sv1, const SemanticVersion &sv2);
bool operator<=(const SemanticVersion &sv1, const SemanticVersion &sv2);

#endif // SEMANTIC_VERSION_H
