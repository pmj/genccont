//
//  DListSpliceTest.m
//  DListSpliceTest
//
//  Created by Laura Dennis on 6/14/13.
//
//

#import "DListSpliceTest.h"
#import "../../src/dlist.h"

struct char_list_item
{
	genc_dlist_head_t head;
	char c;
};
typedef struct char_list_item char_list_item;

static char_list_item
	a = {{}, 'A'},
	b = {{}, 'B'},
	c = {{}, 'C'},
	d = {{}, 'D'},
	e = {{}, 'E'},
	f = {{}, 'F'},
	g = {{}, 'G'};

static genc_dlist_head_t list1, list2;

@implementation DListSpliceTest

- (void)setUp
{
	[super setUp];
    
	genc_dlist_init(&list1);
	genc_dlist_insert_before(&a.head, &list1);
	genc_dlist_insert_before(&b.head, &list1);
	genc_dlist_insert_before(&c.head, &list1);


	genc_dlist_init(&list2);
	genc_dlist_insert_before(&d.head, &list2);
	genc_dlist_insert_before(&e.head, &list2);
	genc_dlist_insert_before(&f.head, &list2);
	genc_dlist_insert_before(&g.head, &list2);
}

- (void)tearDown
{  
	[super tearDown];
}

- (void)testDoNothing
{
 /* We have these 2 lists:
	* 1: -> H1 <-> A <-> B <-> C <-
	* 2: -> H2 <-> D <-> E <-> F <-> G <-
	*/

	genc_dlist_head_t* cur = list1.next;
	STAssertTrue(cur != &list1, @"List is not empty");
	char_list_item* cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'A', @"Expect item A");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'B', @"Expect item B");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'C', @"Expect item C");
	
	cur = cur->next;
	STAssertEquals(cur, &list1, @"Expect end of list");
	
	genc_dlist_head_t* cur2 = list2.next;
	STAssertTrue(cur2 != &list2, @"List is not empty");
	char_list_item* cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'D', @"Expect item D");
	
	cur2 = cur2->next;
	STAssertTrue(cur2 != &list2, @"Shouldn't be at end of list");
	cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'E', @"Expect item E");

	cur2 = cur2->next;
	STAssertTrue(cur2 != &list2, @"Shouldn't be at end of list");
	cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'F', @"Expect item F");

	cur2 = cur2->next;
	STAssertTrue(cur2 != &list2, @"Shouldn't be at end of list");
	cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'G', @"Expect item G");
	
	cur2 = cur2->next;
	STAssertEquals(cur2, &list2, @"Expect end of list");
}

- (void)testSplicing
{
	/* We have these 2 lists:
	 * 1: -> H1 <-> A <-> B <-> C <-
	 * 2: -> H2 <-> D <-> E <-> F <-> G <-
	 *
	 * And we call genc_dlist_splice(A, D, G), we end up with:
	 *
	 * 1: -> H1 <-> A <-> E <-> F <-> B <-> C <-
	 * 2: -> H2 <-> D <-> G <-
	 */
	
	genc_dlist_splice(&a.head, &d.head, &g.head);
	
	genc_dlist_head_t* cur = list1.next;
	STAssertTrue(cur != &list1, @"List is not empty");
	char_list_item* cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'A', @"Expect item A");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'E', @"Expect item E");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'F', @"Expect item F");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'B', @"Expect item B");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'C', @"Expect item C");
	
	cur = cur->next;
	STAssertEquals(cur, &list1, @"Expect end of list");
	
	
	genc_dlist_head_t* cur2 = list2.next;
	STAssertTrue(cur2 != &list2, @"List is not empty");
	char_list_item* cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'D', @"Expect item D");
	
	cur2 = cur2->next;
	STAssertTrue(cur2 != &list2, @"Shouldn't be at end of list");
	cur2_char = genc_container_of(cur2, char_list_item, head);
	STAssertEquals(cur2_char->c, (char)'G', @"Expect item B");
	
	cur2 = cur2->next;
	STAssertEquals(cur2, &list2, @"Expect end of list");

}

- (void)testConcatenate
{
	 /* We have these 2 lists:
	* 1: -> H1 <-> A <-> B <-> C <-
	* 2: -> H2 <-> D <-> E <-> F <-> G <-
	*
	* and we concatenate them genc_dlist_splice(H1, H2, H2)
	* we end up with:
	* 1: -> H1 <-> D <-> E <-> F <-> G <-> A <-> B <-> C <-
	* 2: -> H2 <-
	*/
		
	genc_dlist_splice(&list1, &list2, &list2);
	
	genc_dlist_head_t* cur = list1.next;
	STAssertTrue(cur != &list1, @"List is not empty");
	char_list_item* cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'D', @"Expect item D");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'E', @"Expect item E");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'F', @"Expect item F");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'G', @"Expect item G");	
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'A', @"Expect item A");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'B', @"Expect item B");
	
	cur = cur->next;
	STAssertTrue(cur != &list1, @"Shouldn't be at end of list");
	cur_char = genc_container_of(cur, char_list_item, head);
	STAssertEquals(cur_char->c, (char)'C', @"Expect item C");
	
	cur = cur->next;
	STAssertEquals(cur, &list1, @"Expect end of list");
	
	genc_dlist_head_t* cur2 = list2.next;
	cur2 = cur2->next;
	STAssertEquals(cur2, &list2, @"Expect end of list");

}


@end
