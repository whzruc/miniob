
#include "common/log/log.h"
#include "sql/operator/update_operator.h"
#include "storage/record/record.h"
#include "storage/common/table.h"
#include "storage/trx/trx.h"
#include "sql/stmt/update_stmt.h"


RC UpdateOperator::open()
{
	if(children_.size()!=1){
		LOG_WARN("update operator must has 1 child");
		return RC::INTERNAL;
	}
	Operator *child =children_[0];
	RC rc=child->open();
	if(rc!=RC::SUCCESS){
		LOG_WARN("failed to open child operator: %s",strrc(rc));
		return rc;
	}
	Table *table =update_stmt_->table();
	while(RC::SUCCESS ==(rc =child->next())){
		Tuple *tuple =child->current_tuple();
		if(nullptr ==tuple){
			LOG_WARN("failed to get crrent record: %s",strrc(rc));
			return rc;
		}

		RowTuple *row_tuple =static_cast<RowTuple *>(tuple);
		Record &record =row_tuple->record();
		// 这里我选择改接口来实现类似的效果
		rc=table->update_record(trx_,update_stmt_->value(),update_stmt_->value_index(),&record);

		if(rc!=RC::SUCCESS){
			LOG_WARN("failed to update record: %s",strrc(rc));
			return rc;
		}
	}
	return RC::SUCCESS;
}

RC UpdateOperator::next(){
	return RC::RECORD_EOF;
}
RC UpdateOperator::close(){
	children_[0]->close();
	return RC::SUCCESS;
}