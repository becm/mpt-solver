/*!
 *  define NLS user init parameters
 */
struct mpt_client;
extern int client_init(int , char * const []);
extern int solver_run(struct mpt_client *c);
