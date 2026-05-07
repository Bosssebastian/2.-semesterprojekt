<script lang="ts">
	type Props = {
		open?: boolean;
		children?: import('svelte').Snippet;
	};

	let { open = false, children }: Props = $props();
</script>

{#if open}
	<div data-slot="dialog-root" class="fixed inset-0 z-50">
		{@render children?.()}
	</div>
{/if}
