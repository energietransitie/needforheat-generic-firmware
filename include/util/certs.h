#ifndef CERTS_H
#define CERTS_H

// This is the embedded root CA certificate for api.energietransitiewindesheim.nl.
extern const char isrg_root_pem_start[] asm("_binary_isrg_root_pem_start");
extern const char isrg_root_pem_end[] asm("_binary_isrg_root_pem_end");

// This is the embedded root CA certificate for api.github.com.
extern const char github_root_pem_start[] asm("_binary_github_root_pem_start");
extern const char github_root_pem_end[] asm("_binary_github_root_pem_end");

#endif // CERTS_H
